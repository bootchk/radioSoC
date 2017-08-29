
#pragma once

#include <inttypes.h>

#include "../platformTypes.h"	// OSTime


/*
 * Thin wrapper around RTC device of Nordic nRF52/51.
 * This defines an API that might be portable.
 *
 *
 *
 * Provides:
 * - LongClock 56-bit counter. At resolution of 30uSec, overflows in 67,000 years.
 * - base Counter for Timers (LongClock must be running to use Timer)
 *
 *
 *
 * Units (precision) is ticks of 30uSec (from the crystal LF clock.)
 *
 * Accuracy is the same as for the crystal LF clock (say 20ppm.)
 *
 * Returns the current value of this machines low(?) resolution time source.
 * It bears no relation to:
 * - any other machines time source,
 * - to any other time source on the machine,
 * - or to wall time.
 * In particular, one machine's time source can drift at rate of specified accuracy, in relation to other machine's time source.
 *
 * Resolution:
 * - successive nowTime() calls may return the same value .
 * - successive nowTime() calls may return values whose difference is greater than one tick (no guarantee that this code is fast.)
 * - back-to-back nowTime() calls should not return values differing by much
 *   (but by how much depends on the cpu speed, certainly not greater than MaxTimeout)
 * - successive nowTime() calls spanning greater than (2**56-1)*30uSec  (about 68,000 years) will be incorrect due to 56-bit rollover.
 *
 * Monotonicity:
 * - a nowTime() call result is >= result from a prior call !!!!
 * (unless you call resetToNearZero() .)
 *
 * Timer durations:
 * - min and max specified below
 * Max timeout is (2**24-1)*30uSec ~=8.39minutes
 *
 * Legal call sequences:
 *
 * Normal:  init, getNowTime, startTimer, cancelTimer
 * You can cancel a Timer that has not been started: init, cancelTimer
 *
 * Each timer has a callback that occurs while interrupts are disabled (called from within the ISR.)
 * Here the three Timers are distinguished in the ISR:
 * - First timer is used for low-power sleep functionality
 * - other timers do all their processing within the callback (it should be short.)
 * All the Timers wake up the mcu from sleep.
 *
 * Implementation:
 *
 * Owns private data defined in anon namespace.
 *
 * 24-bit RTC device counter underlies both LongClock and Timers.
 *
 * To insure monotonicity and correctness, uses Lamport's Rule to deal with for internal counter rollover.
 * Failing to use Lamport's Rule or other means, could result:
 * - not monitonic
 * - consecutive results difference near MaxTimeout
 */


/*
 * Type that LongClock yields.  Only 56 bits are valid.
 */
typedef uint64_t LongTime;






class LongClock {
	static const int OSClockCountBits = 24;

public:

	// TODO revisit this, not enforced
	// Device won't reliably cause event for timeouts < 2
	// Some authors use 3 ???
	// TODO revisit Product Spec
	// app_timer used 5, because it had other delays?
	static const unsigned int MinTimeout = 2;

	/*
	 * Called in IRQHandler, with interrupts disabled.
	 */
	static void longClockISR();

	/*
	 * Clock methods.
	 */
	/*
	 * Start does not guarantee clock is running (only that it is started.)
	 * You can start a timer even if clock is not running, but it may delay longer than requested,
	 * until oscillator isRunning and thus isOSClockRunning().
	 *
	 * If you require accurate timer, start a dummy timer before you start an accurate timer.
	 */
	static void start();

	/*
	 * AKA Reset the clock, but not to zero.
	 * Make high order bits zero.
	 * Low order bits are whatever the HW counter is.
	 */
	static void resetToNearZero();

	static LongTime nowTime();
	static OSTime osClockNowTime();	// LSB

	static bool isOSClockRunning();
	static void waitOneTick();
};
