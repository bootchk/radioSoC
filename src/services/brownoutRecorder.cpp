
#include "brownoutRecorder.h"

#include "../services/customFlash.h"

// platform lib
#include <drivers/powerComparator.h>


namespace {

BrownoutCallback callback1 = nullptr;
BrownoutCallback callback2 = nullptr;
BrownoutCallback callback3 = nullptr;

/*
 * Record a trace (phase, reason, timesincelastsleep)
 * as returned by callbacks
 * in three consecutive locations in flash
 */
void recordTraceToFlashAtIndex(FlagIndex index)  {
	// assume if one callback is valid, they all are
	if (callback1 != nullptr) {
		CustomFlash::writeWordsAtIndex(index, callback1(), callback2(), callback3());
#ifdef OLD
		CustomFlash::tryWriteIntAtIndex(index, callback1());
		CustomFlash::tryWriteIntAtIndex((FlagIndex)(index+1), callback2());
		CustomFlash::tryWriteIntAtIndex((FlagIndex)(index+2), callback3());
#endif
	}
}

/*
 * Record trace at first unwritten location, or not at all.
 */
void recordTraceToFlash() {
	if (! CustomFlash::isWrittenAtIndex(CustomFlash::BrownoutTrace1Index))  recordTraceToFlashAtIndex(CustomFlash::BrownoutTrace1Index);
	else if (! CustomFlash::isWrittenAtIndex(CustomFlash::BrownoutTrace2Index)) recordTraceToFlashAtIndex(CustomFlash::BrownoutTrace2Index);
}

}	// namespace


void BrownoutRecorder::registerCallbacks(
		BrownoutCallback a,
		BrownoutCallback b,
		BrownoutCallback c) {
	callback1 = a;
	callback2 = b;
	callback3 = c;
}

// OLD parameter uint32_t faultAddress
void BrownoutRecorder::recordToFlash() {
	/*
	 * Since for some power supplies, the system may repeatedly brownout and POR,
	 * and since flash is not writeable more than once,
	 * only write PC if not written already.
	 */

	/*
	 * !!! First must counteract HW lock of flash during brownout.
	 * Disable comparator so it doesn't trigger again when we clear the event.
	 */
	PowerComparator::disable();
	PowerComparator::clearPOFEvent();
	// assert NVMC is not HW locked against writes

	/*
	 * The rest takes power, and up to 300uSeconds per word written to flash.
	 * It might not succeed in writing to flash, since power is failing
	 */

	// Record what callbacks return e.g. Phase of algorithm
	recordTraceToFlash();

	// Also record faultAddress (another indication of location in algorithm
	// Not much info, usually just the WFE i.e. usually brownout in sleep
	// CustomFlash::tryWriteIntAtIndex(BrownoutPCIndex, faultAddress);


	/*
	 * This is not designed rigorously for the continuation.
	 * The PowerComparator is left in a state different from entry and will not catch further brownouts.
	 * A possible continuation is to infinite loop.
	 * Another possible clean continuation is to stick to the algorithm
	 * It is reasonable to want a clean continuation,
	 * because the detected brownout is only potential; might not become a real BOR reset.
	 * That is, power might recover.
	 *
	 * Any continuation might resume brownout detection (after hysteresis)
	 * and we might get here again.
	 */
}
