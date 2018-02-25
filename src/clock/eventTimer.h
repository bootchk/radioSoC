
#pragma once

#include "../platformTypes.h"   // OSTime



/*
 * A timer that does not use interrupts but just generates events.
 * Typically events connected to PPI to trigger tasks.
 */

class EventTimer {
public:
	static void start(OSTime timeout);
	static void stop();
	static uint32_t* getEventRegisterAddress();
};
