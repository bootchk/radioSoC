
#pragma once

// embeddedMath
#include <types.h>	// DeltaTime, LongTime





/*
 * Knows Duration on an assumed clock (the LongClock.)
 *
 * Uses LongClock static methods.  LongClock instance must be initialized first.
 */
class ClockDuration {
public:
	/*
	 * Not require futureTime later than now, returns 0 if it is.
	 * Requires laterTime less than MaxDeltaTime from now
	 */
	static DeltaTime clampedTimeDifferenceFromNow(const LongTime laterTime);


	/*
	 * Require pastTime earlier than now.
	 * Returns zero if pastTime later than now.
	 *
	 * Don't use this when pastTime should be asserted to be an earlier time. (use elapsed())
	 */
	static DeltaTime clampedTimeDifferenceToNow(LongTime earlierTime);


	/*
	 * Smallest positive difference of givenTime from now, or now from givenTime.
	 *
	 * - givenTime may be in the past or future
	 * - givenTime is less than MAX_DELTA_TIME from now.
	 */
	static DeltaTime timeDifferenceFromNow(LongTime givenTime);

	/*
	 * Elapsed DeltaTime since earlierTime, referenced to now.
	 * Asserts if earlierTime is after now.
	 * Asserts if difference between earlierTime and now is larger than max DeltaTime.
	 *
	 * Use this when LongTime was simply recorded in the past (not the result of math.)
	 */
	static DeltaTime elapsed(LongTime earlierTime);

};
