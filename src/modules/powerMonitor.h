#pragma once

#include <drivers/powerComparator.h>  // PowerThreshold

/*
 * Adaptor to PowerComparator.
 *
 * This knows how to use PowerComparator for two purposes:
 * - measuring power
 * - monitoring for brownout
 *
 * The two purposes require coordination:
 * brownout monitoring must be turned off while measuring power.
 *
 * !!! Modal.
 * enterBrownoutDetectionMode() enters the mode.
 * Currently no way to leave the mode.
 *
 * BrownoutDetection gives a BrownoutWarning.
 *
 * BrownoutWarning is distinct from actual BOR (BrownoutReset).
 * BOR comes at an even lower voltage than BrownoutThreshold.
 * BOR IS a reset, i.e. execution stops and restarts.
 * The voltage level of BOR is about 1.7V, but varies a little among chip instances.
 *
 * BrownoutWarning has a hysteresis:
 * once the event occurs, it does not occur again until after voltage has climbed back above BrownoutThreshold
 * AND you have called a voltage getting function (which restarts detection.)
 *
 * It is possible to get many BrownoutWarning's without an actual BOR.
 *
 * See BrownoutRecorder.
 *
 * Power requirements:
 * PowerComparator uses 4uA when enabled AND (HFCLK is running.)
 * !!! So when sleeping, brownout might not be detected:
 * - sleeping w/ radio on: HFCLK (HFXO) is running and brownout is detected
 * - sleeping w/o radio: brownout not detected.
 */

class PowerMonitor {

public:

	/*
	 * NRF5x family constrains lowest possible threshold:
	 * - nrf51: NRF_POWER_POFTHR_V21
	 * - nrf52: NRF_POWER_POFTHR_V17
	 * But you can change this to higher value.
	 *
	 * For testing on NRF52DK, change to V23 since debugger needs higher voltage for comm to target.
	 */


	/*
	 * Sets mode.
	 *
	 * Actual brownout detection does not start until after first call to isVddGreater...
	 *
	 * Enables at lowest threshold that POFCON can detect.
	 * Which may be lower/higher than you want.
	 *
	 * Current design: never leave the mode.
	 */
	static void enterBrownoutDetectMode();

	/*
	 * No matter what the mode, disable brownout detection.
	 *
	 * Not change the mode, and brownout detection might be enabled later
	 * (as soon as a call to isVddGreaterxxx determines that Vcc is above 2.1 again.
	 */
	static void disableBrownoutDetection();

	/*
	 * Simple tests of Vdd thresholds.
	 *
	 * These have side effects on the PowerFailureComparator device resource:
	 * they leave it disabled.
	 */
	// These are portable nrf51 and nrf52
	static bool isVddGreaterThanThreshold(PowerThreshold);
	//static bool isVddGreaterThan2_3V();
	//static bool isVddGreaterThan2_5V();
	//static bool isVddGreaterThan2_7V();
#ifdef NRF52
	// This is the highest one that NRF52 supports (it has more granularity, but not much more range.)
	//static bool isVddGreaterThan2_8V();
#endif
};
