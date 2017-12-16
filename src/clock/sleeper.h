
#pragma once

#include "longClock.h"


/*
 *
 * Responsibilities:
 * - sleep the system (wake on timeout or other event.)
 * - know reason for waking.
 * - know OSTime from OSClock
 * - know and enforce max timeout a sane app would ask for (currently an assertion, should be exception.)
 *
 * Uses a Clock/Timer.
 *
 * Sleeping puts mcu to idle, low-power mode.
 * Note much power management is automatic by nrf52.
 * E.G. when sleep, all unused peripherals are powered off automatically.
 */


/*
 * Reasons mcu woke.
 * Interrupts wake the mcu, and ISR's set these.
 * TODO make this exhaustive (there are a few other sleeps implemented.)
 *
 * Every sleeper must handle each of these reasons.  (enum class gives warning if not.)
 */
enum class ReasonForWake{
	Cleared = 2,	// The null state, not set by any ISR
	MsgReceived,
	SleepTimerExpired,
	CounterOverflowOrOtherTimerExpired,
	BrownoutWarning,
	HFClockStarted,
	LFClockStarted,
	Unknown			// An ISR was called but it found no expected events.  Not used?
} ;





class Sleeper {
public:
	// Public because passed to radio so it can hook IRQ into it
	static void msgReceivedCallback();


	/* maxSaneTimeout: max timeout a sane app should ask for. */
	static void setSaneTimeout(OSTime maxSaneTimeout);

	/*
	 * Sleep until any system event, and set a Timer that generates a waking event after timeout ticks.
	 * That is, sleep no more than timeout.
	 * Could sleep less if any event wakes us.
	 * ISR's set ReasonForWake.
	 * This may override ReasonForWake with higher priority reasons.
	 */
	static void sleepUntilEventWithTimeout(OSTime);

	/*
	 * Sleep a duration.
	 * Ignore any events that wake us before duration.
	 */
	static void sleepDuration(OSTime);


	/* Cancel Timer that would generate waking event. */
	static void cancelTimeout();

	/*
	 * Sleep until one specific event, ignoring all other events.
	 */
	static void sleepUntilEvent(ReasonForWake);

	// Not in-lined, used by external libraries
	static void setReasonForWake(ReasonForWake);	// not always used, internally
	static ReasonForWake getReasonForWake();
	static void clearReasonForWake();

	/*
	 * Specific to SyncSleeping: when sleeping without radio and not expecting any other events e.g. HFClockStarted
	 */
	static bool isWakeForTimerExpired();
};


