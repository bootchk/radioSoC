
#include "mcuSleep.h"

// platform lib nRF5x
#include <drivers/mcu.h>


void MCUSleep::untilAnyEvent() {
	MCU::sleepUntilEvent();
}

void MCUSleep::untilInterrupt() {
	MCU::sleepUntilInterrupt();
}
