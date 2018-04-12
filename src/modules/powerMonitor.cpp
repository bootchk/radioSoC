#include "powerMonitor.h"


// Used by brownoutCallback
#include "../services/brownoutRecorder.h"
#include <drivers/powerComparator.h>
#include "../clock/sleeperObs.h"


/*
 * Implementation notes.  See header for API notes.
 *
 * Much of the complexity of implementation is multiplexing:
 * There is only one PowerComparator, so we have to alternate (multiplex) its use between:
 * - simple Vcc measurement
 * - brownout detection.
 * Might be easier to use ADC for simple Vcc measurement.
 *
 * !!! Note interrupts handled in POWER_CLOCK_IRQHandler() see elsewhere.
 * It is hardwired to call BrownoutRecorder and call PowerMonitor::disableBrownoutDetection
 */

namespace {

/*
 * No instance of PowerComparator. Pure class, all class methods
 */

bool _brownoutDetectionMode = false;

/*
 * Return result of compare Vdd to threshold.
 * Leaves:
 * - device disabled
 * - event cleared
 *
 */
bool testVddGreaterThanThresholdThenDisable() {
	bool result;

	// Require threshold to be set
	// But some thresholds (V21) have value zero on nrf51

	// clear event so enabling might reset it
	PowerComparator::clearPOFEvent();

	PowerComparator::enable();

	/*
	 * Testing shows that POFCON does not generate event immediately,
	 * at least on nrf52.  Requires a very short delay.
	 */
	PowerComparator::delayForPOFEvent();

	/*
	 * Event indicates Vdd less than threshold.
	 * Invert: return true if Vdd greater than threshold.
	 */
	result = ! PowerComparator::isPOFEvent();
	PowerComparator::disable();
	PowerComparator::clearPOFEvent();
	return result;
}


/*
 * Enable brownoutDetection if enableBrownoutDetection() was called previously (i.e. entered mode.)
 */
void tryEnableBrownoutDetection() {
	// not require not detecting already, this may be superfluous
	if (_brownoutDetectionMode) {

		// set lowest possible threshold the POFCON supports on family
		PowerComparator::setBrownoutThresholdAndDisable();

		// assert isEventClear()
		PowerComparator::enableInterrupt();
		PowerComparator::enable();
	}
	// assert _brownoutDetectionMode=>PowerComparator is actively detecting brownout and will interrupt
}



/*
 * Agnostic of brownout detection.
 * Requires PowerComparator to be not in use for brownout detection.
 */
bool isVddGreaterThanThreshold(PowerThreshold threshold) {
	bool result;

	// get a new measurement on different threshold
	PowerComparator::setThresholdAndDisable(threshold);
	result = testVddGreaterThanThresholdThenDisable();

	return result;
}

/*
 * This takes care to maintain the alternate use of POFCON: brownout detect.
 *
 * It briefly disables brownout interrupt, then restores it.
 */
bool isVddGreaterThanThresholdWithBrownoutDetection(PowerThreshold threshold) {
	bool result;

	// disable brownout detection briefly while we use device for other purpose
	PowerComparator::disableInterrupt();

	// get result to be returned
	result = isVddGreaterThanThreshold(threshold);

	/*
	 * Restore PowerComparator to brownout detect if result is true.
	 * Here we assume  BrownoutThreshold is the lowest possible threshold,
	 * so that result == true means Vcc is above BrownoutThreshold.
	 * OW (result == false) leave detection off so we don't continually issue BrownoutWarning.
	 */
	// !!! not isVddGreaterThanThreshold(PowerMonitor::BrownoutThreshold) because it is time consuming
	if (result) {
		tryEnableBrownoutDetection();
	}

	/*
	 * Assert result valid.
	 * Assert brownout detection enabled if enableBrownoutDetection() was called previously
	 * else POFCON disabled and threshold is indeterminate.
	 */
	return result;
}


/*
 * Called from ISR upon brownout.
 * Interrupts disabled, should be short.
 */
void brownoutCallback(void) {

	BrownoutRecorder::recordToFlash();

#ifdef NOT_USED
	// Generally, brownout during sleep, so not much use to get faultAddress
	BrownoutRecorder::recordToFlash(faultAddress);
#endif

	/*
	 * Proceed and wait for actual BOR
	 */
	/*
	 *  Signal, if we were sleeping.
	 */
	// TODO prioritize
	// Sleeper::setReasonForWake(ReasonForWake::BrownoutWarning);

	// assert brownout ISR disables further brownout detection
}



}  // namespace





void PowerMonitor::initBrownoutCallback() {
	PowerComparator::registerBrownoutCallback(brownoutCallback);
}



void PowerMonitor::enterBrownoutDetectMode() {
	_brownoutDetectionMode = true;
	// Detection not effected until first call to isVddGreaterThan2xxx
}


void PowerMonitor::disableBrownoutDetection() {
	PowerComparator::disableInterrupt();
	PowerComparator::disable();
	// !!! Not affect the mode:  _brownoutDetectionMode = false;
}

/*
 * !!! Side effect: enable brownout detection.
 */
bool PowerMonitor::isVddGreaterThanThreshold(PowerThreshold threshold) {
	return isVddGreaterThanThresholdWithBrownoutDetection(threshold);
}
#ifdef OLD
bool PowerMonitor::isVddGreaterThan2_3V() { return isVddGreaterThanThresholdWithBrownoutDetection(NRF_POWER_POFTHR_V23); }
bool PowerMonitor::isVddGreaterThan2_5V() { return isVddGreaterThanThresholdWithBrownoutDetection(NRF_POWER_POFTHR_V25); }
bool PowerMonitor::isVddGreaterThan2_7V() { return isVddGreaterThanThresholdWithBrownoutDetection(NRF_POWER_POFTHR_V27); }
#ifdef NRF52
bool PowerMonitor::isVddGreaterThan2_8V() { return isVddGreaterThanThresholdWithBrownoutDetection(NRF_POWER_POFTHR_V28); }
#endif
#endif


