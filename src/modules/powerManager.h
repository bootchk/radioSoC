
#pragma once

/*
 * Levels.  The count of ranges is one more.
 */
enum class VoltageRange { BelowUltraLow, UltraLowToLow, LowToMedium, MediumToHigh, HighToExcess, AboveExcess};

/*
 * Understands:
 * - power levels and ranges
 *
 * excessVoltage:
 * The power supply may be unregulated (solar)
 * and capable of system damaging voltages (e.g. 4.8V exceeding mcu Vmax of 3.6V.)
 *
 * Depends on some device that read system Vcc (Vdd).
 * Voltage levels on some charge storage (capacitor) indicates power level.
 *
 * Brownout is:
 * - real
 * - OR detected
 * Detected brownout only means that the mcu will soon suffer real brownout (a reset, BOR.)
 * Typically, the handler for detected brownout:
 * - quits app (if power supply is a simple battery w/o harvested charging)
 * - OR alters the app behaviour to use less power (if power supply is harvested and may return to higher voltage.)
 */
class PowerManager {

public:
	static void init();

	/*
	 * Simple enter a mode.
	 * Does not actually start detection until after a call to one of the isPower...() functions.
	 */
	static void enterBrownoutDetectMode();
	// leaveBrownoutDetecMode not implemented

	/*
	 * Levels
	 */
	static bool isPowerExcess();	// Above Vmax of chip 3.6V
	static bool isPowerAboveUltraHigh();	//
	static bool isPowerAboveHigh();
	static bool isPowerAboveMedium();
	static bool isPowerAboveLow();
	static bool isPowerAboveUltraLow();

	/*
	 * Ranges.
	 *
	 * Expensive since it calls a sequence of the isPower...() functions
	 */
	static VoltageRange getVoltageRange();
};
