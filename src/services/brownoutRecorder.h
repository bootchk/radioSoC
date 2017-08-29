
#pragma once

#include <inttypes.h>

typedef unsigned int (*BrownoutCallback)();

/*
 * Knows how to write important info to flash when brownout happens.
 *
 * Pure class, no instances.
 *
 * An IRQ might call it.
 */


class BrownoutRecorder {
public:
	/*
	 * Write to flash if not already written:
	 *  - result of callback if callback registered
	 *  - else faultAddress
	 *
	 * Called in a brownout state (EVENTS_POFWARN is set)
	 */
	//OLD static void recordToFlash(uint32_t faultAddress);
	static void recordToFlash();

	/*
	 * Register function that returns important information at time of brownout.
	 */
	static void registerCallbacks(BrownoutCallback, BrownoutCallback, BrownoutCallback);
};

