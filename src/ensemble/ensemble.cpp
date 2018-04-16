
#include <cassert>


#include "ensemble.h"

#include "../radio/radio.h"

// platform lib
#include <drivers/oscillators/hfClock.h>
#include <drivers/powerSupply.h>


#include "../clock/clockFacilitator.h"

// temp, for measuring varying startup duration
#include "../clock/longClock.h"
#include "../services/logger.h"

#include "../radio/radioData.h"

#include "../radioUseCase/radioUseCase.h"


namespace {

	// has-a
	RadioUseCase* activeUseCase = nullptr;

	// !!! Some parameters of use case can be changed and apply immediately to ensemble.
}


#ifdef obsolete
void Ensemble::init(MsgReceivedCallback aCallback) {

	assert(! HfCrystalClock::isRunning());	// xtal not running
	Radio::setMsgReceivedCallback(aCallback);
}
#endif


void Ensemble::setRadioUseCase(RadioUseCase* aRadioUseCase){
	activeUseCase = aRadioUseCase;
	assert(activeUseCase!=nullptr);

	/*
	 * Configuration changes when use case changes.
	 *
	 * Multiprotocol i.e. different use case means configuration is changed.
	 *
	 * Platform dependent but generally:
	 *    - stays configured until mcu is POR.
	 *    - OR until radio power is toggled (to the default config.)
	 */
	Radio::configureForSleepSync();

	// Apply persistent but changeable on the fly parameters
	activeUseCase->applyToRadio();
}



// The only member that needs configuration is Radio
bool Ensemble::isConfigured(){ return Radio::isConfiguredForSleepSync(); }


bool Ensemble::isLowPower() {
#ifdef RADIO_POWER_IS_REAL
	return ((!Radio::isPowerOn()) && !Radio::hfCrystalClock->isRunning());
#else
	// Radio will be low power if not in use
	return ((!isRadioInUse()) && !HfCrystalClock::isRunning());
#endif
}

/*
 * Methods simply to the Radio.
 */
bool Ensemble::isRadioInUse() 			   { return Radio::isInUse(); }
BufferPointer Ensemble::getBufferAddress() { return Radio::getBufferAddress(); }
bool Ensemble::isPacketCRCValid()          { return Radio::isPacketCRCValid(); }
unsigned int Ensemble::getRSSI()          { return Radio::receivedSignalStrength(); }


#ifdef OBSOLETE
/*
 * Startup must be called before StartReceiving or Transmit
 */
void Ensemble::startup() {

	// enable this first so it has time to ramp up
	// FIXME DCDC
	/*
	 * This should be conditionally compiled for whether board has DCDC components?  e.g. Waveshare
	 * The behaviour of the power supply when board has no LC components is not specified.
	 *
	 * This should check whether Vcc > 2.1V.
	 * Nordic docs state DCDC should not be used in that condition.
	 */
	// until fixed, commented out
	// DCDCPowerSupply::enable();

#ifdef RADIO_POWER_IS_REAL
	Radio::powerOn();
	// Some platforms require config after powerOn()
	Radio::configurePhysicalProtocol();
#else
	// Radio stays powered and configured.
#endif

	/*
	 * Timing varies by board, since different crystal models used.
	 * NRF52DK: 11 ticks == 360uSec
	 * Waveshare nRF51: 40 ticks == 1200uSec
	 */
	/* Debugging
	LongTime startTime = LongClock::nowTime();
	*/

	/*
	 * OLD
	 * Formerly, startup required varying duration.
	 * Now, startup takes a constant duration.
	 *
	 * HfCrystalClock::startAndSleepUntilRunning();
	 */
	// TODO symbolic constant that depends on the board
	ClockFacilitator::startHFClockWithSleepConstantExpectedDelay(40);

	/* Debugging
	logInt(LongClock::nowTime() - startTime);
	log("<hfclock start\n");
	*/

	assert(Radio::isConfiguredForSleepSync());
}
#endif


void Ensemble::shutdown() {
	HfCrystalClock::stop();

#ifdef RADIO_POWER_IS_REAL
	Radio::powerOff();
#else
	// If not in use, will enter standby low-power mode automatically
	assert(! Radio::isInUse());
#endif

	// disable because Vcc may be below what DCDCPowerSupply requires
	DCDCPowerSupply::disable();
}



void Ensemble::startReceiving() {
	/*
	 * Not log to flash here, since it takes too long.
	 */

	// TODO should this be in caller?
	// OLD syncSleeper.clearReasonForWake();

	assert(Radio::isPowerOn());
	Radio::receiveStatic();
	assert(Radio::isInUse());

	/*
	 * SyncSleeper will clearReasonForWake().
	 * Thus there is a low probablity race here.
	 * Any message that arrives before SyncSleeper clears reason might be lost.
	 * But it is low probability since there is a rampup time (40-140 uSec, i.e. 700-2100 instruction cycles) for the radio
	 * to go from disabled to active.
	 * We almost always will sleep before the radio is able to receive.
	 */
}



void Ensemble::stopReceiving() {
	if (Radio::isInUse()) {
		Radio::stopReceive();
	}
	assert(!Radio::isInUse());
}


void Ensemble::transmitStaticSynchronously(){
	assert(Radio::isPowerOn());
	assert(Radio::isConfiguredForSleepSync());

	Radio::transmitStaticSynchronously();
}

