#pragma once

#include <inttypes.h>

#include "radioTypes.h"

// platform lib e.g. nRF5x
//#include <drivers/radio/radio.h>
//#include <drivers/hfClock.h>
//#include <drivers/nvic.h>
//#include <drivers/powerSupply.h>
//#include <drivers/radio/types.h>	// BufferPointer

// uses LongClock to timestamp.
#include "../clock/longClock.h"



// XXX enum class
typedef enum {
	Receiving,
	Transmitting,
	Idle,
	PowerOff
}RadioState;


enum class RadioPowerState {
	On,
	Off
};




/*
 * High level driver for radio peripheral
 *
 * Understands collaboration of low-level devices:
 * - HfClock and RadioDevice
 * - DCDCPower and RadioDevice
 *
 * Not understand registers, i.e no dependence on nrf.h
 *
 * Understands wireless protocol:
 * - configures device for protocol
 * - abstracts protocol state from device state
 * - abstracts away the task/event architecture of device
 * - implements interrupt RX vs. spinning TX
 * - understands half-duplex (RX exclusive of TX)
 *
 * Protocol is defined by:
 * - constants for lengths, channels, bitrate
 * - behaviour
 * -- no acks xmitted as in ESB
 * -- all units use one address
 *
 * Singleton, all static class methods.
 */
/*
 * Algebra of valid call sequences:
 *
 *  Typical:
 *    init(), assert(isConfigured), getBufferAddress(), <put payload in buffer> transmitStaticSynchronously(),
 *            receiveStatic(), sleepUntilEventWithTimeout(),  if isDisabledState() getBufferAddress(); <get payload from buffer>,
 *            resetAndConfigure(), ...
 *
 *  init() must be called once after mcu POR:
 *    mcu.reset(), init(), mcu.reset(), init(), ...
 *
 *  configure() is called by init() and configuration is generally static.
 *  FUTURE implement resetAndConfigure to redo the whole configuration.
 *  Note that POR state of radio is POWER==1 i.e. recently reset.
 *  Must toggle POWER to reset the configuration.
 *  POWER does not really affect current consumption of radio!!!
 *
 *  Configuration can only be changed when radio is not in use i.e. disabled:
 *    receiveStatic(), configureTXPower() is invalid
 *    init(), configureTXPower() is valid
 *
 *  transmitStaticSynchronously blocks, but receiveStatic does not.  If reasonForWake is not MsgReceived,
 *  you must stopReceive() before next radio operation:
 *    receiveStatic(), sleepUntilEventWithTimeout(),
 *      if sleeper.reasonForWake() != MsgReceived then stopReceive()
 *
 *  When reasonForWake is MsgReceived, buffer is full and radio is ready for next transmit or receive.
 *  A packet received can have an invalidCRC.
 *  A packet received having validCRC can be garbled (when count of bit errors greater than CRC can detect.)
 *     receivedStatic(), sleepUntilEventWithTimeout(), if sleeper.reasonForWake() == MsgReceived then
 *        getBuffer(), if isPacketCRCValid() then <use received buffer>,
 *        transmitStaticSynchronously()
 *
 *  You can transmit or receive in any order, but Radio is half-duplex:
 *    init(), , receiveStatic(), ...stopReceive(),
 *       receiveStatic(), ..., stopReceive
 *       transmitStaticSynchronously(), ...
 *
 *	Radio isDisabledState() is true after certain other operations:
 *	   init(), assert(isDisabledState())
 *	   transmitStaticSynchronously(), assert(isDisabledState())
 *	   stopReceive(), assert(isDisabledState())
 *	   receiveStatic(), sleepUntilEventWithTimeout(), if sleeper.reasonForWake() == MsgReceived assert(isDisabledState())
 *
 *	Radio isDisabledState() is false at least for a short time after a receive()
 *	but if a packet is received, is true
 *	    receiveStatic(), assert(! isDisabledState()), ..<packet received>, assert(isDisabledState())
 *
 *  Radio is has a single buffer, non-queuing.  After consecutive operations, first contents of buffer are overwritten.
 */
class Radio {

public:

	// Radio knows and uses HfCrystalClock, DCDCPowerSupply, Nvic

	// Define protocol lengths

	/*
	 * Fixed: all payloads same size.
	 *
	 * Device config: not transmit S0, LENGTH, S1
	 * Buffer not include S0, LENGTH, S1 fields.
	 *
	 * Must match length of Message class (struct).
	 * Which for SleepSync is 1 MessageType + 6 MasterID + 3 offset + 1 WorkPayload
	 */
	static const uint8_t FixedPayloadCount = 11;

	/*
	 * bitrate in megabits [1,2]
	 */
	static const uint8_t MegabitRate = 2;


	// Length of transmitted physical layer address (not part of payload.)
	// All units have same physical address so
	static const uint8_t LongNetworkAddressLength = 4;	// 1 byte preamble, 3 bytes base
	static const uint8_t MediumNetworkAddressLength = 3;	// 1 byte preamble, 2 bytes base
	static const uint8_t ShortNetworkAddressLength = 2;	// 1 byte preamble, 1 bytes base

	/*
	 * Frequency index in [0..100]
	 *
	 * Freq fixed to one of 3 BT advertising channels, to avoid WiFi interference.
	 *
	 * 2, 26, 80 : freq = 2.4gHz + FrequencyIndex
	 * e.g. 80 yields 2480 kHz
	 *
	 * The BT channel ID's are much different.
	 * BT is 2402 to 2480
	 *
	 * TODO why not use above 2480?
	 */
	static const uint8_t FrequencyIndex = 80;



	static void radioISR();

	/*
	 * Set callback for all physical messages received.
	 * Callback is usually to another protocol layer, not necessarily to app layer.
	 */
	static void setMsgReceivedCallback(void (*onRcvMsgCallback)());


	/*
	 * These are generic.
	 * They might be impotent (mocked) on some platforms.
	 * Might not be necessary on Nordic.
	 */
	static void powerOn();
	static void powerOff();
	static bool isPowerOn();


	/*
	 * Configure protocol: SleepSync.
	 * Other configuration required before each session (data buffer).
	 * Not generic: specific to SleepSync protocol.
	 */
	static void configureForSleepSync();
	static bool isConfiguredForSleepSync();
	/*
	 * Configure parameters of physical protocol: freq, addr, CRC, bitrate, etc
	 */
	static void configurePhysicalProtocol();



	static void configureXmitPower(TransmitPowerdBm power);
	static TransmitPowerdBm getXmitPower();
	// Runtime validity check of OTA value
	static bool isValidXmitPower(TransmitPowerdBm power);

	//static void resetAndConfigure();


	// Power off peripherals the radio depends on.
	static void shutdownDependencies();


	/*
	 * Abort: may come at any time.
	 * Generally, when power is failing.
	 *
	 * Put radio in condition that it will use little power.
	 * Make not xmitting or receiving.
	 * Then the chip will unpower it automagically.
	 */
	static void abortUse();

	/*
	 *  is radio receiving or transmitting?
	 *  !!! The chip automatically powers radio power.
	 */
	static bool isInUse();

	//static bool isDisabledState();
	static bool isEnabledInterruptForPacketDoneEvent();

	// FUTURE DYNAMIC static void getBufferAddressAndLength(uint8_t** handle, uint8_t* lengthPtr);
	// Can't define in-line, is exported
	static BufferPointer getBufferAddress();


	// Static: buffer owned by radio, of fixed length
	static void transmitStaticSynchronously();	// blocking
	static void spinUntilXmitComplete();
	static void stopXmit();

	static void receiveStatic();
	// Only returns true once, until after startReceive again.
	static bool isReceiveInProgress();
	static void clearReceiveInProgress();
	static void spinUntilReceiveComplete();
	static void stopReceive();

	static bool isEnabledInterruptForMsgReceived();
	static bool isEnabledInterruptForEndTransmit();

#ifdef DYNAMIC
	static void transmit(BufferPointer data, uint8_t length);
	static void transmitSynchronously(BufferPointer data, uint8_t length);
	static void receive(BufferPointer data, uint8_t length);

	static void setupXmitOrRcv(BufferPointer data, uint8_t length);
#endif

	/*
	 * Attributes of most recently received packet.
	 */
	static bool isPacketCRCValid();
	static LongTime timeOfArrival();
    static unsigned int receivedSignalStrength();


// FUTURE to anon namespace
private:
	static void setupFixedDMA();

	/*
	 * reset configuration.
	 *
	 * Only needed if configuration is to be changed other than at POR time, when we already know radio is reset
	 */
	//static void reset();

	/*
	 * Is radio ready for OTA, i.e. rampup is done.
	 * Not needed since we use shortcuts.
	 */
	//static void spinUntilReady();

	static void dispatchPacketCallback();

	static void setupInterruptForMsgReceivedEvent();

	static void startXmit();
	static void startRcv();

	static void startRXTask();
	static void startTXTask();
	static void startDisableTask();
	static void spinUntilDisabled();

	static bool isEventForMsgReceivedInterrupt();
	static void clearEventForMsgReceivedInterrupt();
	static void enableInterruptForMsgReceived();
	static void disableInterruptForMsgReceived();
	static void disableInterruptForEndTransmit();

	static void transmitStatic();
};
