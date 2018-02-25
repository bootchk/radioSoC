
#pragma once

#include <inttypes.h>

/*
 * Flashes LED via task/events w/o using interrupts
 */
class LEDFlasherTask {
public:
	static void init(uint32_t pin );
	static void flashLEDByAmount(unsigned int amount);
};
