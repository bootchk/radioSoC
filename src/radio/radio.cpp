
#include <cassert>

#include "radio.h"
#include "radioData.h"

// platform lib e.g. nRF5x
#include <drivers/radio/radio.h>

#include <drivers/oscillators/hfClock.h>
#include <drivers/nvic/nvic.h>
#include <drivers/powerSupply.h>





// Debugging
// TODO pass this into radio and only call it if non-null
// LEDService ledLogger2;


/*
 * Implementation notes:
 *
 * Interrupts are used.  Alternative is to poll (see RadioHead.)
 * Now, interrupt on receive.  FUTURE: interrupt on xmit.
 *
 * Uses "configuration registers" of the RADIO peripheral(section of the datasheet).
 * AND ALSO "task registers" of the "Peripheral Interface"(section of the datasheet)
 * to the RADIO (for tasks, events, and interrupts.)
 * See device/nrf52.h for definitions of register addresses.
 * Each task and event has its own register.
 *
 * Not keeping our own state (radio peripheral device has a state.)
 *
 * !!! Note this file has no knowledge of registers (nrf.h) , see radioLowLevel and radioConfigure.c
 */




/*
 * single instance of data, but not class members
 * namespace RadioData
 */
RadioDevice RadioData::device;
void (*RadioData::aRcvMsgCallback)() = nullptr;
LongTime RadioData::_timeOfArrival;
RadioState RadioData::state;
volatile uint8_t RadioData::radioBuffer[Radio::FixedPayloadCount];




namespace {

extern "C" {

__attribute__ ((interrupt ("RADIO_IRQ")))
void
RADIO_IRQHandler()  { Radio::radioISR(); }

} // extern C

} // namespace




void Radio::radioISR(void)
{
	// We only expect an interrupt on packet received

    if (isEventForMsgReceivedInterrupt())
    {
    	/*
    	 * Timestamp packet ASAP.
    	 * For every packet, including those with CRC errors.
    	 */
    	RadioData::_timeOfArrival = LongClock::nowTime();

    	assert(RadioData::state == Receiving);	// sanity

    	clearEventForMsgReceivedInterrupt();

    	// ledLogger2.toggleLED(2);	// debug: LED 2 show every receive

    	/*
    	 * Call next layer.
    	 * For SleepSyncAgent calls Sleeper::msgReceivedCallback() which sets reasonForWake
    	 */
    	assert(RadioData::aRcvMsgCallback!=nullptr);
    	RadioData::aRcvMsgCallback();
    }
    else
    {
        /*
         * Probable programming error.
         * We were awakened by a radio event other than the only enabled interrupt 'MsgReceived'
         * (which on some platforms is radio DISABLED)
         * Brownout and bus faults (DMA?) could come while mcu is sleeping.
		 * Invalid op code faults can not come while mcu is sleeping.
         */
    	// FUTURE handle more gracefully by just clearing all events???
    	// FUTURE recover by raising exception and recovering by reset?
    	assert(false);
    }
    // We don't have a queue and we don't have a callback for idle
    assert(!isEventForMsgReceivedInterrupt());	// Ensure event is clear else get another unexpected interrupt
    // assert Sleeper::reasonForWake != None
}


LongTime Radio::timeOfArrival() { return RadioData::_timeOfArrival; }



/*
 * Private routines that isolate which event is used for interrupt on End Of Transmission.
 * The choices are END or DISABLED.
 */
#ifdef PACKET_DONE_INTERRUPT
bool Radio::isEventForEOTInterrupt() { return device.isPacketDone(); }
void Radio::clearEventForEOTInterrupt() { device.clearPacketDoneEvent(); }
void Radio::enableInterruptForEOT() { device.enableInterruptForPacketDoneEvent(); }
void Radio::disableInterruptForEOT() { device.disableInterruptForPacketDoneEvent(); }
bool Radio::isEnabledInterruptForEOT() { return device.isEnabledInterruptForPacketDoneEvent(); }
#else
// DISABLED event
bool Radio::isEventForMsgReceivedInterrupt() {
	/*
	 * !!! The radio stays in the disabled state, even after the event is cleared.
	 * So this is not:  device.isDisabled().
	 */
	return RadioData::device.isDisabledEventSet();
}
void Radio::clearEventForMsgReceivedInterrupt() { RadioData::device.clearDisabledEvent(); }

/*
 * In interrupt chain, disable in two places: nvic and device
 * Assume PRIMASK is always clear to allow interrupts.
 */
void Radio::enableInterruptForMsgReceived() {
	assert(!RadioData::device.isDisabledEventSet());	// else interrupt immediately???
	Nvic::enableRadioIRQ();
	RadioData::device.enableInterruptForDisabledEvent();
}
void Radio::disableInterruptForMsgReceived() {
	Nvic::disableRadioIRQ();
	RadioData::device.disableInterruptForDisabledEvent();
}
bool Radio::isEnabledInterruptForMsgReceived() {
	return RadioData::device.isEnabledInterruptForDisabledEvent();	// FUTURE && nvic.isEnabledRadioIRQ();
}

/*
 * Currently, semantics are same as for MsgReceived.
 */
void Radio::disableInterruptForEndTransmit() { disableInterruptForMsgReceived(); }

bool Radio::isEnabledInterruptForEndTransmit() { return RadioData::device.isEnabledInterruptForDisabledEvent(); }
#endif

/*
 * The register is read only.
 * Should only be called when packet was newly received (after an IRQ or event indicating packet done.)
 */
bool Radio::isPacketCRCValid(){
	// We don't sample RSSI
	// We don't use DAI device address match (which is a prefix of the payload)
	// We don't use RXMATCH to check which logical address was received
	// (assumed environment with few foreign 2.4Mhz devices.)
	// We do check CRC (messages destroyed by noise.)
	return RadioData::device.isCRCValid();
}


#ifdef OBSOLETE
void Radio::dispatchPacketCallback() {
	// Dispatch to owner callbacks
	if ( ! wasTransmitting ) { aRcvMsgCallback(); }
	// No callback for xmit
}
#endif



void Radio::configureForSleepSync()
{
	// Not require radio device power on (that bit just resets config to default.)

	// assert radio is configured to device reset defaults, which is non-functional.
	configurePhysicalProtocol();

	/*
	 * not ensure callback not nullptr: must set else radio might receive but not relay packet on
	 * not ensure HFXO is started, which radio needs
	 */
	assert(isConfiguredForSleepSync());
}


void Radio::setMsgReceivedCallback(void (*onRcvMsgCallback)()){
	RadioData::aRcvMsgCallback = onRcvMsgCallback;
}



void Radio::abortUse() {
	// Disable interrupt required for startDisableTask()
	disableInterruptForMsgReceived();
	startDisableTask();
	/*
	 * Might be a delay until radio is disabled.
	 * Here, we don't wait.
	 * EVENTS_DISABLED may get set soon.
	 */
	RadioData::state = Idle;
}


bool Radio::isInUse() { return ! RadioData::device.isDisabledState(); }

void Radio::spinUntilDisabled() {
	/*
	 * Assert:
	 * - we started the task DISABLE
	 * - or we think is in reset state (just after RADIO->POWER toggled or POR)
	 * - or a shortcut from active state such as TX will succeed
	 * Not to be used to wait for a receive, because it may fail
	 */
	/*
	 *  Implementation: Wait until state == disabled  (not checking for event.)
	 *
	 *  See data sheet.
	 *  For (1Mbit mode, TX) to disabled delay is ~6us.
	 *  RX to disabled delay is ~0us
	 *  delay from reset (toggle POWER) to disabled is ???
	 */
	while (!RadioData::device.isDisabledState()) ;
}

#ifdef OLD
void Radio::resetAndConfigure() {
	reset();
	configurePhysicalProtocol();
}



void Radio::reset() {
	// Does not require !device.isPowerOn()), it can be left enabled.

	// require Vcc > 2.1V (see note below about DCDC)

	// radio requires HFXO xtal clock, not the HFRC hf clock
	assert(hfCrystalClock->isRunning());

	// !!! toggling the bit called 'POWER' is actually just reset and does not affect power
	// The chip manages power to radio automatically.
	device.setRadioPowered(false);
	device.setRadioPowered(true);

	// Assert there is no delay: state is immediately disabled?
	// There is a delay using the DISABLE Task from RX or TX
	spinUntilReady();

	// assert if it was off, the radio and its registers are in initial state as specified by datasheet
	// i.e. it needs to be reconfigured

	// assert HFCLK is on since radio uses it

	state = Idle;

	// !!! Configuration was lost, caller must now configure it
	assert(!isEnabledInterruptForMsgReceived());
	assert(device.isDisabledState());		// after reset, initial state is DISABLED
}


void Radio::spinUntilReady() {
	// BUG: WAS disable(); spinUntilDisabled();  but then I added an assertion to disable();
	/*
	 * Wait until power supply ramps up.
	 * Until then, device registers are indeterminate?
	 *
	 * This is the way RadioHead does it.
	 * But how do you start a task when registers are indeterminate?
	 */
	device.clearDisabledEvent();
	device.startDisablingTask();
	spinUntilDisabled();
}
#endif


void Radio::shutdownDependencies() {
	// not require on; might be off already

	/*
	 * require disabled: caller must not power off without disabling
	 * because we also use the disabled state for MsgReceived (with interrupts)
	 * The docs do not make clear whether the device passes through
	 * the DISABLED state (generating an interrupt) when powered off.
	 */
	assert(!isInUse());

	/* OLD
	device.setRadioPowered(false);
	// not ensure not ready; caller must spin if necessary
	*/

	HfCrystalClock::stop();
	// assert hf RC clock resumes for other peripherals

	RadioData::state = PowerOff;
	// assert HFXO off, or will be soon
	// assert the chip has powered radio off
}



#ifdef OBSOLETE
// Get address and length of buffer the radio owns.
void Radio::getBufferAddressAndLength(uint8_t** handle, uint8_t* lengthPtr) {
	*handle = staticBuffer;
	*lengthPtr = PayloadCount;
}
#endif




/*
 * Per Nordic docs, must setup DMA each xmit/rcv
 * Fixed: device always use single buffer owned by radio
 */
void Radio::setupFixedDMA() {
	RadioData::device.configurePacketAddress(getBufferAddress());
}

/*
 * radioBuffer is an array, return its address by using its name on right hand side.
 */
BufferPointer Radio::getBufferAddress() { return RadioData::radioBuffer; }



void Radio::transmitStaticSynchronously(){
	// ledLogger2.toggleLED(4);	// Dual purpose LED4: invalid or xmit
	disableInterruptForEndTransmit();	// spin, not interrupt

	// Lag for rampup, i.e. not on air immediately
	transmitStatic();
	// FUTURE: sleep while xmitting to save power
	spinUntilXmitComplete();
	// assert xmit is complete and device is disabled
}


// Private, called only above
void Radio::transmitStatic(){
	RadioData::state = Transmitting;
	setupFixedDMA();
	startXmit();
	// not assert xmit is complete, i.e. asynchronous and non-blocking
}


void Radio::receiveStatic() {
	RadioData::state = Receiving;
	setupFixedDMA();
	setupInterruptForMsgReceivedEvent();
	startRcv();
	// assert will get IRQ on message received
}

bool Radio::isReceiveInProgress() {
	return RadioData::device.isReceiveInProgressEvent();
}

#ifdef FUTURE
// When catching receive in progress at timeout time
void Radio::spinUntilReceiveComplete() {
	// Same as:
	spinUntilXmitComplete();
}
#endif

void Radio::clearReceiveInProgress() {
	RadioData::device.clearReceiveInProgressEvent();
}

#ifdef DYNAMIC
void Radio::transmit(volatile uint8_t * data, uint8_t length){
	state = Transmitting;
	setupXmitOrRcv(data, length);
	startXmit();
	// not assert xmit is complete, i.e. asynchronous and non-blocking
};


void Radio::receive(volatile uint8_t * data, uint8_t length) {
	wasTransmitting = false;
	setupXmitOrRcv(data, length);
	startRcv();
}
#endif

/*
 * Starting task is final step.
 * Start a task on the device.
 * Require device is configured, including data and DMA.
 */
void Radio::startRXTask() {
	RadioData::device.clearMsgReceivedEvent();	// clear event that triggers interrupt
	RadioData::device.startRXTask();
}

void Radio::startTXTask() {
	RadioData::device.clearEndTransmitEvent();	// clear event we spin on
	RadioData::device.startTXTask();
}

void Radio::startDisableTask() {
	/*
	 * !!! Require caller to disable interrupt.
	 */
	assert(!isEnabledInterruptForEndTransmit());
	RadioData::device.clearDisabledEvent();
	RadioData::device.startDisablingTask();
	/*
	 * Disabling takes time.  Disabled event will come soon.
	 * But it is not correct to set internalState = Idle until then.
	 */
}



/*
 * Usually (but not required),
 * device is configured: shortcuts, packetAddress, etc.
 * and buffer is filled.
 */
void Radio::setupInterruptForMsgReceivedEvent() {
	// Clear event before enabling, else immediate interrupt
	clearEventForMsgReceivedInterrupt();
	enableInterruptForMsgReceived();
}


#ifdef DYNAMIC
void Radio::setupXmitOrRcv(volatile uint8_t * data, uint8_t length) {
	/*
	 * Assert
	 * is configured: shortcuts, packetAddress, etc.
	 */
	device.setShortcutsAvoidSomeEvents();
	device.configurePacketAddress(data);
	//device.configurePacketLength(length);
	enableInterruptForEOT();
	clearEventForEOTInterrupt();
}
#endif





// task/event architecture, these trigger or enable radio device tasks.

void Radio::startXmit() {
	assert(RadioData::device.isDisabledState());  // require, else behaviour undefined per datasheet
	startTXTask();
	// assert radio state will soon be TXRU and since shortcut, TX
}

void Radio::startRcv() {
	assert(RadioData::device.isDisabledState());  // require, else behaviour undefined per datasheet
	startRXTask();
	/*
	 * assert: (since shortcuts)
	 * 1. radio state will soon be RXRU
	 * 2. ramp up delay incurred of 130us (nrf52) or 40us (nrf52)
	 * 3. radio state RXIDLE (shortcut)
	 * 4. radio state RX (shortcut)
	 */
}

void Radio::stopReceive() {
	/*
	 *  assert radio state is:
	 * RXRU : aborting before ramp-up complete
	 * or RXIDLE: on but never received start preamble signal
	 * or RX: in middle of receiving a packet
	 * or DISABLED: message was received and RX not enabled again
	 */

	disableInterruptForMsgReceived();
	//isReceiving = false;

	if (! RadioData::device.isDisabledState()) {
		// was receiving and no messages received (device in state RXRU, etc. but not in state DISABLED)
		RadioData::device.startDisablingTask();
		// assert radio state soon RXDISABLE and then immediately transitions to DISABLED
		spinUntilDisabled();
		// DISABLED event was just set, clear it now before we later enable interrupts for it
		RadioData::device.clearMsgReceivedEvent();
	}

	/*
	 * The above checked a state returned by the radio,
	 * which experience shows can differ from the event for the state.
	 * So here we explicitly clear the event to ensure it corresponds with radio state.
	 */
	RadioData::device.clearMsgReceivedEvent();

	RadioData::state = Idle;

	assert(RadioData::device.isDisabledState());
	assert(!RadioData::device.isDisabledEventSet());	// same as MsgReceivedEvent
	assert(!RadioData::device.isEnabledInterruptForDisabledEvent()); // "
	// assert no interrupts enabled from radio, for any event
}

void Radio::stopXmit() {
	// Rarely used: to abort a transmission, generally radio completes a xmit
	// FUTURE
}

/*
 * This takes a few msec, depends on bit count and bit rate.
 * Say 100 bits at 1Mbit/sec is 0.1mSec
 *
 * Alternatively, enable interrupt and sleep, but radio current dominates mcu current anyway?
 *
 * Nothing can prevent xmit from completing?  Even bad configuration or HFCLK not on?
 */
// FUTURE use interrupt on xmit.
void Radio::spinUntilXmitComplete() {
	//assert isTransmitting

	/*
	 * Implementation:
	 * Radio state flows (via TXDISABLE) to DISABLED.
	 * Wait for DISABLED state (not the event.)
	 * For xmit, we do not enable interrupt on EVENTS_DISABLED.
	 */
	spinUntilDisabled();	// Disabled state means xmit done because using shortcuts

	// EVENTS_DISABLED is set, leave it set but clear it before enabling interrupt on it

	RadioData::state = Idle;
}



unsigned int Radio::receivedSignalStrength() {
	return RadioData::device.receivedSignalStrength();
}



