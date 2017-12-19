#pragma once

#include "../platformTypes.h"	// OSTime



/*
 * Facilitates (coordinates) starting of various clocks.
 * Understands/hides dependencies and startup times.
 * For higher level clocks:
 * - LongClock (extended, counting, LFClock).  Starts the dependencies and sleeps until running.
 * - HfClock (just an oscillator, no counting).  No dependencies but delays or sleeps until running.
 *
 *
 * LongClock and Timers depend on a RTC (Counter and CompareRegisters) depends on LowFrequencyClock.
 * Starting requires IRQ enabled.
 *
 * Radio requires HfClock.
 * I am not sure how Radio behaves if HfClock is not stable.
 */
class ClockFacilitator {
public:
	/*
	 * Start LongClock and insure it is running so Timers can be started.
	 * Blocks.
	 * Varied duration.
	 * Cannot be used with SD since uses interrupt on blocked device PowerClock.
	 */
	static void startLongClockWithSleepUntilRunning();

	/*
	 * Start LongClock without ensuring it is running.
	 */
	static void startLongClockNoWaitUntilRunning();

	static bool isLongClockRunning();


	/*
	 * Start HfClock.
	 * Sleep a constant time that is expected to be
	 * enough time for HFXO to be running stably.
	 */
	static void startHFClockWithSleepConstantExpectedDelay(OSTime delay);


	/*
	 * Start HfClock.
	 * Sleep a varying time, until conditoin: HFXO running stably.
	 * Sleep during startup to avoid energy consumption.
	 * Does not return until running, but is low power until then.
	 * HfCrystalClock takes about 1.2, 0.36 mSec to run stable depending on board design.
	 */
	static void startHFXOAndSleepUntilRunning();
};
