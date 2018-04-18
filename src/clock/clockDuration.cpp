
#include "clockDuration.h"

#include "timeMath.h"


// implementation uses lib radioSoC: LongClock and Timer
#include <clock/longClock.h>


#include <cassert>





DeltaTime ClockDuration::clampedTimeDifferenceFromNow(LongTime futureTime) {
	DeltaTime result = TimeMath::clampedTimeDifference(futureTime, LongClock::nowTime()); // Coerced to 32-bit with possible loss
	// Already asserted: assert(result < MaxDeltaTime);
	return result;
}




DeltaTime ClockDuration::clampedTimeDifferenceToNow(LongTime pastTime) {
	DeltaTime result =  TimeMath::clampedTimeDifference(LongClock::nowTime(), pastTime); // Coerced to 32-bit with possible loss
	// Already asserted: assert(result < MaxDeltaTime);
	return result;
}





DeltaTime ClockDuration::timeDifferenceFromNow(LongTime givenTime) {
	LongTime now = LongClock::nowTime();
	LongTime result;

	// Subtract past time from larger future time, else modulo math gives a large result
	if (now < givenTime)
		result = givenTime - now;	// Unsigned, modulo math
	else
		result = now - givenTime;

	return TimeMath::convertLongTimeToOSTime(result);
}

/*
 * Elapsed DeltaTime since earlierTime, referenced to now.
 * Asserts if earlierTime is after now.
 * Asserts if difference between earlierTime and now is larger than max DeltaTime.
 *
 * Use this when LongTime was simply recorded in the past (not the result of math.)
 */
DeltaTime ClockDuration::elapsed(LongTime earlierTime) {
	LongTime now = LongClock::nowTime();

	// Require earlier time to left on practically infinite linear timeline
	assert(now >= earlierTime);

	LongTime diff = now - earlierTime;

	// Will assert if overflows DeltaTime
	return  TimeMath::convertLongTimeToOSTime(diff);
}
