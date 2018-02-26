
#pragma once

#include <inttypes.h>

/*
 * Flashes LED via task/events w/o using interrupts
 */
class LEDFlasherTask {
public:
	/*
	 * Connect EventTimer event to PinTask task via EventToTaskSignal (PPI)
	 */
	static void init(uint32_t pin );

	/*
	 * Turn LED on and start timer to turn it off.
	 */
	static void flashLEDByAmount(unsigned int amount);
};
