
#include <cassert>

#include "sleeper.h"

// implementation
#include <timeMath.h>
#include "timer.h"

// platform lib
#include <drivers/mcu.h>


namespace {

const TimerIndex SleepTimerIndex = First;	// Must not be used elsewhere


/*
 * Shared user of LongClock
 * Exclusive use of timer[SleepTimerIndex]
 */

OSTime maxSaneTimeout = Timer::MaxTimeout;	// defaults to max a Timer allows

/*
 * Volatile because ISR's set it.
 * However, ARM is atomic on 32-bit words
 * so we don't need a critical section.
 */
volatile ReasonForWake reasonForWake = ReasonForWake::Cleared;





/*
 * Callback from RTC_IRQ which dispatches on event: sleep timer compare register expire, other timer compare register expire, and overflow.
 * This callback is called for all those events.
 * !!! May be other Timers which wake mcu and whose callbacks are called as well as this, but those callbacks do not set reasonForWake.
 *
 * Here we set flag that main event loop reads so that the user of this Timer (who is sleeping) knows reason for wake.
 * Since there are two concurrent devices (radio and counter), there is a race to set reasonForWake.
 * Here we prioritize.
 *
 * This callback is from within IRQHandler with nested interrupts precluded, so keep short or schedule a task, queue work, etc.
 *
 * We pass address around, so names can be C++ mangled
 */

void timerIRQCallback(TimerInterruptReason reason) {
	/*
	 * Prioritize concurrent reasonForWake, compare new reason with existing reason
	 */
	switch(reason) {
	case SleepTimerCompare:

		switch(reasonForWake) {
		case ReasonForWake::Cleared:
		case ReasonForWake::Unknown:
		case ReasonForWake::BrownoutWarning:
		case ReasonForWake::CounterOverflowOrOtherTimerExpired:
			// Higher priority reason
			reasonForWake = ReasonForWake::SleepTimerExpired;
			// TODO assert that Timer current Count - Timer starting count == timeout
			break;
		case ReasonForWake::MsgReceived:
			// Do not overwrite highest priority: MsgReceived
			break;
		case ReasonForWake::SleepTimerExpired:
			// Timer was started again before handling/clearing previous expiration.
			MCU::breakIntoDebuggerOrHardfault();

		case ReasonForWake::HFClockStarted:
		case ReasonForWake::LFClockStarted:
			// Timer was started without clearing previous reasonForWake??
			MCU::breakIntoDebuggerOrHardfault();
		}
		break;

	case OverflowOrOtherTimerCompare:
		/*
		 * Awakened, but not for First Timer.
		 * Overflow and OtherTimer can come even if a reason is already set,
		 * since the interrupt is always enabled for Overflow,
		 * and interrupt for other timers is periodically enabled.
		 * XXX simpler to use separate peripheral for other timers.
		 */
		switch(reasonForWake) {
		case ReasonForWake::Cleared:
			reasonForWake = ReasonForWake::CounterOverflowOrOtherTimerExpired;
			break;

		case ReasonForWake::BrownoutWarning:
		case ReasonForWake::Unknown:
		case ReasonForWake::MsgReceived:
		case ReasonForWake::SleepTimerExpired:
		case ReasonForWake::CounterOverflowOrOtherTimerExpired:
			// Reason is already higher priority
			break;
		case ReasonForWake::HFClockStarted:
		case ReasonForWake::LFClockStarted:
			assert(false);	// Design does not user timer while HF or LF clock is starting
		}
	}
	// assert reasonForWake is not Cleared
}

} // namespace






void Sleeper::setSaneTimeout(OSTime maxAppTimeout) {
	maxSaneTimeout = maxAppTimeout;
}

void Sleeper::sleepUntilEventWithTimeout(OSTime timeout) {
	// units are ticks, when RTC has zero prescaler: 30uSec

	// TODO should we be clearing, or asserting (rely on caller to clear, because of races?)
	clearReasonForWake();

	/*
	 * OLD design: if less than minimum required by Timer implementation. Don't sleep, but set reason for waking.
	 * reasonForWake = ReasonForWake::SleepTimerExpired;
	 *
	 * NOW: if timeout is small, event and interrupt will still occur (forced by implementation.)
	 */


	/*
	 * Sanity of SleepSync: never sleeps longer than two SyncPeriodDuration
	 */
	// TODO this should be a clamp, or throw
	assert(timeout <= maxSaneTimeout );

	/*
	 * oneshot timer CAN trigger before the startTimer() returns.
	 *
	 * Timer implementation must not oversleep, since we are not using WDT.
	 */
	Timer::start(
			SleepTimerIndex,
			timeout,
			timerIRQCallback);

	/*
	 * Timer may already have expired and set EventRegister.
	 * Or EventRegister may be set for other reasons.
	 * When EventRegister is set, this will not sleep.
	 * To insure a sleep, you must either insure EventRegister is not set, or call this in a loop,
	 * since MCU::sleep() will clear EventRegister.
	 */
	MCU::sleep();
	/*
	 * awakened by event: received msg or timeout or other event.
	 * !!! Other timer expirations may wake us.
	 * But other timer expirations won't call self's callback.
	 */

	/*
	 * If timer expired, timer is already stopped.
	 * Else, stop it to ensure consistent state.
	 * Note that for our timer semantics, it is safe to stop a timer that it not started,
	 * but not safe to start a timer that is already started.
	 */
	Timer::cancel(SleepTimerIndex);

	/*
	 * Cases:
	 * - never slept OR slept then woke and:
	 * -- RTC IRQ set reasonForWake to SleepTimerExpired
	 * -- RTC or RADIO IRQ handler set reasonForWake in [Timeout, MsgReceived)
	 * -- RTC IRQ for another Timer and reasonForWake is still none
	 * -- unexpected event woke us and reasonForWake is still None
	 *
	 * In all cases, assert timer is stopped (so using our timer semantics, it can be started again.)
	 *
	 * !!! Cannot assert that timeout amount of time has elapsed: other clock events may wake early.
	 */
}


void Sleeper::cancelTimeout(){
	Timer::cancel(SleepTimerIndex);
}



void Sleeper::sleepUntilSpecificEvent(ReasonForWake reason){
	/*
	 * sleep until IRQ signals started event.
	 * !!! Other interrupts (brownout, clock overflow, led Timer 2 etc. may wake the sleep.)
	 * Other interrupts may increase time between start() and sleep().
	 * The max start time is 360uSec NRF52,
	 * so there should be plenty of time to get asleep before interrupt occurs.
	 */

	while (Sleeper::getReasonForWake() != reason) {
		/*
		 * !!! Event must come, else infinite loop.
		 * Ignore other events from clock and other sources
		 * e.g. timers for flashing LED's (whose interrupts will occur, but returns to this thread.)
		 */
		MCU::sleep();
		// TODO examine other reasons here and log abnormal ones such as brownout
	}
}


void Sleeper::sleepDuration(OSTime duration) {


	//void SyncSleeper::sleepUntilTimeout(DeltaTime timeout)

	LongTime endingTime = LongClock::nowTime() + duration;
	OSTime remainingTimeout = duration;

	while (true) {

		// assert(remainingTimeout < ScheduleParameters::MaxSaneTimeout);

		/*
		 * !!! Just sleep, with no assertions on power.
		 */
		Sleeper::sleepUntilEventWithTimeout(remainingTimeout);

		if (Sleeper::isWakeForTimerExpired()) {
			break;	// while true, assert time timeout has elapsed.
		}
		else {
			// reasonForWake is not TimerExpired
			remainingTimeout = TimeMath::clampedTimeDifferenceFromNow(endingTime);
			// continue next loop iteration
		}
		/*
		 * waking events spend time, is monotonic and will eventually return 0
		 * and Sleeper::sleepUntilTimeout will return without sleeping and with reasonForWake==Timeout
		 */
	}
	// assert timeout amount of time has elapsed
}


/*
 * Called by RadioIRQ.
 * Not assert packet is valid.
 */
void Sleeper::msgReceivedCallback() {
	/*
	 * If msg arrives after main read reasonForWake and before it stopped the receiver,
	 * this reason will go ignored and msg lost.
	 *
	 * If msg arrives immediately after a timeout but before main has read reasonForWake,
	 * the msg will be handled instead of the timeout.
	 */
	reasonForWake = ReasonForWake::MsgReceived;
}


/*
 * reasonForWake  is an enum class and compile time checked valid
 */
ReasonForWake Sleeper::getReasonForWake() { return reasonForWake; }
void Sleeper::setReasonForWake(ReasonForWake reason) { reasonForWake = reason; }
void Sleeper::clearReasonForWake() { reasonForWake = ReasonForWake::Cleared; }


bool Sleeper::isWakeForTimerExpired() {
	/*
	 * assert woken from a sleep w/o radio
	 * not assert that timer went off: it may yet set a new reason at any moment.
	 * We don't clear reasonForWake just because an IRQ could set it at any moment.
	 */
	bool result = false;

	// Expect wake by timeout, not by msg or other event
	switch( getReasonForWake() ) {
	case ReasonForWake::SleepTimerExpired:
		result = true;
		// assert time specified by timeoutFunc has elapsed.
		break;

	case ReasonForWake::CounterOverflowOrOtherTimerExpired:
		/*
		 * Normal
		 * But do not end sleep.
		 */
		break;

	case ReasonForWake::MsgReceived:
		/*
		 * Abnormal: Radio should be off.
		 * But do not end sleep.
		 */
		//LogMessage::logUnexpectedMsg();
		break;

	case ReasonForWake::Unknown:
		/*
		 * Unexpected, probably a bug.
		 * Radio is not in use can't receive
		 * Woken by some interrupt (event?) not in the design.
		 * But do not end sleep.
		 */
		//LogMessage::logUnexpectedWakeReason();
		break;
	case ReasonForWake::Cleared:
		// Should be impossible, woken w/o any ISR setting reasonForWake
		//assert(false);
		// TODO log this
		break;
	case ReasonForWake::HFClockStarted:
	case ReasonForWake::LFClockStarted:
		// Impossible, not starting clock now
		assert(false);
		break;
	case ReasonForWake::BrownoutWarning:
		/*
		 * Possible.  But we are already sleeping in lowest power.
		 * Nothing more we can do but keep sleeping and check power before the next sync step.
		 */
		break;
	}
	return result;
	// Returns whether timeout time has elapsed i.e. whether to stop sleeping loop
}

