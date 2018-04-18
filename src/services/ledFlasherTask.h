
#pragma once

#include "../platformTypes.h"	// MaxTimeout

#include <inttypes.h>

/*
 * Flashes LED via task/events w/o using interrupts
 */
class LEDFlasherTask {
private:
	/*
	 * Conversion factor from FlashAmount to TimerTick.
	 * Ticks for 32kHz clock are 0.030 mSec
	 * Conversion factor of 20  gives a .6 mSec flash, which is barely visible in indoor room light.
	 */
	static const unsigned int TicksPerFlashAmount = 20;

	/*
	 * Min and max FlashAmount
	 */
	static const unsigned int MinFlashAmount = 1;

public:
	/*
	 * Timer constrains max.
	 * Timer may be 32-bit, i.e. 16M
	 * Which yields (16M/20)*0.0006 = 50s
	 */
	static const unsigned int MaxFlashAmount = MaxTimeout / TicksPerFlashAmount;

public:
	/*
	 * Connect EventTimer event to PinTask task via EventToTaskSignal (PPI)
	 */
	static void init(uint32_t pin );

	/*
	 * Turn LED on and start timer to turn it off.
	 */
	static void flashLEDByAmount(unsigned int amount);

	/*
	 * Convert from unit::MinFlashes to units::Ticks
	 */
	static unsigned int amountInTicks(unsigned int amountInMinFlashes);
};
