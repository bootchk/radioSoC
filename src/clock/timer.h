
#pragma once

#include "../platformTypes.h"   // OSTime

/*
 * Enum Timer instances.
 */
// Future: better class with + operator and use it for iterating
// TODO enum class
enum TimerIndex {
	First =0,
	Second
};

/*
 * Reason for interrupt.
 * Users of Timer that sleep on interrupt (WFE) may want to know when wake but timer not expired.
 */
enum TimerInterruptReason {
	OverflowOrOtherTimerCompare,
	SleepTimerCompare
};

// Type of function called back when there is an interrupt from RTC
typedef void (*TimerCallback)(TimerInterruptReason);

/*
 * How many Timers this device supports (with compare registers).
 * Depends on device, here 3 is compatible with nRF51 and nRF52, certain revisions?
 */
static const unsigned int CountTimerInstances = 2;



/*
 * Timers:
 * - 24-bit
 * - one-shot
 * At resolution of 30uSec, max timeout is 5 seconds.
 * !!! Timers are shorter than the LongClock.
 *
 * Requires LongClock isRunning.
 */
class Timer {
private:
	/*
	 * Stop is called internally when Timer has expired OR when it is canceled.
	 * The public API is "Cancel".
	 */
	static void stop(TimerIndex index);

	/*
	 * "expired" is an internal state.
	 * Timer can expire when the underlying CompareRegister has not fired.
	 */
	static void expire(TimerIndex index);
	static void unexpire(TimerIndex index);
	static bool isExpired(TimerIndex index);

	static void configureCompareRegisterForTimer(TimerIndex index, OSTime timeout);



public:

	// Not support timeouts longer than compare register
	static const unsigned int MaxTimeout = 0xFFFFFF;

	// Called by LongClock.start()
	static void initTimers();

	/*
	 * Called from the IRQ handler.
	 * Many events may have occurred (clock overflow, and many compare register matches)
	 */
	static void timerISR();

	static void start(
			TimerIndex index,	// [0:2]
			OSTime timeout, // [0:0xffffff]
			TimerCallback onTimeoutCallback);
	/*
	 * Cancel
	 */
	static void cancel(TimerIndex index);
	static bool isStarted(TimerIndex index);


	/*
	 * Action for state change from unexpired to expired.
	 */
	static void handleExpiration(TimerIndex index);
};
