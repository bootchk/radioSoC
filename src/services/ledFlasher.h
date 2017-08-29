#pragma once

#include "../clock/timer.h"	// MaxTimeout
#include "../modules/ledService.h"



/*
 * Flash an LED
 *
 * Basic features:
 * - start a flash (returns immediately and flash ends later without further calls)
 * - understands amount barely enough for human to see
 * - duration is a parameter 'amount' which is in units of the minimim visible flash
 *
 *
 * Low power:
 * - use Timer to turn it off, so can sleep after calling flashLED
 *
 * Use TimerService which uses an interrupt and callback that are very short.
 * Anyway, it might affect other interrupt servicing.
 *
 * Constraints:
 * - duration is max limited by implementation of Timer (24-bit)
 * - duration is min limited by implementation of Timer ( can't set a timeout less than say 2 ticks.)
 * - duration is also min limited to at least what is visible
 * - a request to flash while LED is already being flashed (ON) is ignored
 */

/*
 * Uses pure classes (device facades).
 * Requires:
 *  Timer is init
 *  assert(LongClock::isOSClockRunning());
 *  require LedLogger initialized???
 *  LEDService initialized
 */

class LEDFlasher {

private:
	/*
	 * Conversion factor from FlashAmount to TimerTick.
	 * Ticks for 32kHz clock are 0.030 mSec
	 * Conversion factor of 20  gives a .6 mSec flash, which is barely visible in indoor room light.
	 */
	static const unsigned int TicksPerFlashAmount = 20;


public:
	/*
	 * Min and max FlashAmount
	 */
	static const unsigned int MinFlashAmount = 1;

	/*
	 * Timer constrains max.
	 * Timer may be 32-bit, i.e. 16M
	 * Which yields (16M/20)*0.0006 = 50s
	 */
	static const unsigned int MaxFlashAmount = Timer::MaxTimeout / TicksPerFlashAmount;



	static void init();

	/*
	 * No effect if ordinal out of range defined by boards.h
	 */

	static void flashLEDMinimumVisible(unsigned int ordinal);

	/*
	 * Flash at least minimal visible time, but less than MaxFlashAmount.
	 *
	 * Units are multiples of least visible time.
	 */
	static void flashLEDByAmount(unsigned int ordinal, unsigned int amount);
};
