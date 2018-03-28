
#include <cassert>

#include "timer.h"

// Implementation
#include "longClock.h"

// platform lib
#include <drivers/nvic/nvic.h>
#include <drivers/clock/compareRegister.h>
#include <drivers/clock/compareRegArray.h>


namespace {
/*
 * RTC device has three compare registers, each generating event/interrupt.
 * All the events are handled by one ISR (RTCx_ISRHandler)
 * This class owns a facade on the counter registers, and a knows a callback for each's interrupt.
 * Callback is also used as a flag for 'started'
 */


TimerCallback timerCallback[2];

/*
 * _isInUse does not imply not expired
 */
bool _isInUse[2];

/*
 * Flag for recent expiration.
 * false does not imply _isInUse
 * true does not imply _isInUse ?
 */
bool _expired[2];


#ifdef TODO
/*
 * !!!! CompareRegisters are constant (the facade is constant, the HW registers are of course writeable.)
 *
 * Parameters of compareRegisters are fixed by hw design of platform (defined by macros.)
 *
 * No need to init CompareRegister, they are constructed const.
 *
 * This does not guarantee the state of the hw compare registers
 * (but typically, all are in POR reset state i.e. disabled.)
 */
const CompareRegister compareRegisters[2] = {
		CompareRegister(NRF_RTC_EVENT_COMPARE_0, NRF_RTC_INT_COMPARE0_MASK, 0),
		CompareRegister(NRF_RTC_EVENT_COMPARE_1, NRF_RTC_INT_COMPARE1_MASK, 1)
};
#endif

} // namespace



/*
 * A Timer is built upon a CompareRegister.
 * Since a CompareRegister has limitations,
 * do more here to accommodate the limitations.
 * This is a concern of the Timer, not of the CompareRegister.
 *
 * Should be no references to platform here.
 */
void Timer::configureCompareRegisterForTimer(TimerIndex index, OSTime timeout){
	// require event disabled?

	OSTime beforeCounter = LongClock::osClockNowTime();
	/*
	 * Interrupts are not disabled.
	 * The counter may continue running while servicing interrupts.
	 * Thus currentCount can get stale, and we must accommodate.
	 */
	/*
	 * RTC is 24-bit timer.
	 * Value is computed in 32-bit math.
	 * We don't need need modulo 24-bit math (mask with 0xFFFFFF)
	 * because the HW of the comparator only reads the lower 24-bits (effectively masks with 0xFFFFFF).
	 * But the values set and get might have ones in upper 8-bits.
	 * Can only assert(nrf_rtc_cc_get() == newCompareValue);
	 */
	unsigned int newCounterValue = beforeCounter + timeout;

	/*
	 * Setting timeout and enabling interrupt must be close together,
	 * else counter exceeds compare already, and no interrupt till much later after counter rolls over.
	 */
	compareRegisters[index].set(newCounterValue);

	OSTime afterCounter = LongClock::osClockNowTime();

	/*
	 * If newCounterValue is not far in the future, CompareRegister will not generate event (or interrupt)
	 * (or it will be one Counter full late) because of HW limitations.
	 *
	 * Compare to NRF_SDK app_timer.c
	 */
	if (((afterCounter - beforeCounter) + LongClock::MinTimeout ) > timeout) {
		/*
		 * CompareRegister might not generate event.
		 * It might (and then this will be repeated/superfluous, but not undone.)
		 */
		// Mark the timer expired already (the small timeout is already over.)
		Timer::expire(index);
		/*
		 * Pend interrupt.
		 * Since the RTCx_IRQ is always enabled, this will generate immediate jump to ISR.
		 * ISR will see the expired timer, and handle it, even though no event from compare register.
		 */
		Nvic::pendLFTimerInterrupt();
	}
	else {
		/*
		 * Guaranteed that CompareRegister will generate event and interrupt.
		 */
	}

	/*
	 * Compare match event might already have happened.
	 * When we enableInterrupt, CompareRegister will generate interrupt
	 * when compare match event happens, or if already set.
	 */
	compareRegisters[index].enableInterrupt();


	/*
	 * Assert: an event and interrupt have been generated already
	 * OR they WILL be generated by the HW.
	 * Most importantly, the ARM EventRegister is OR will be set,
	 * else a subsequent sleep may sleep forever.
	 */
	/*
	 * There is no race to return here.
	 * The caller's continuation is often: sleep until interrupt.
	 * The interrupt can occur at any time here.
	 * The caller must sleep by first WFE (without clearing EventRegister),
	 * and since MCU EventRegister might already be set, won't sleep.
	 * The interrupt typically COULD come in as little as 3 ticks, or 3*30 = 90uSec, which allows about 1440 instructions.
	 */
}






void Timer::timerISR() {

	/*
	 * Source events are "compare register matched counter"
	 * First just determine which Timers are expired because their CompareRegister fired.
	 */
	if ( compareRegisters[First].isEvent() ) {
		compareRegisters[First].disableInterruptAndClearEvent();	//  early
		expire(First);
	}
	if ( compareRegisters[Second].isEvent() ) {
		compareRegisters[Second].disableInterruptAndClearEvent();
		expire(Second);
	}


	/*
	 * Second handle all expired Timers.
	 * Some may have expired without their CompareRegister firing (for short timeout, via an interrupt pended i.e. forced.)
	 */

	if (isExpired(First)) {
		handleExpiration(First);
	}
	else {
		/* Not used for RTC Task design */
#ifndef TASKS
		/*
		 * When we get here, the First Timer has NOT expired but the RTC IRQ was called.
		 * Thus it must be another Timer (compare register match) or Counter overflow.
		 * First Timer is unique: used for a sleep loop.
		 * If it is active, it will have woken by whatever event generated this interrupt.
		 * Pass the callback the reason for wake, so it can sleep again.
		 */
		if ( Timer::isStarted(First) ) {
			timerCallback[First](OverflowOrOtherTimerCompare);
		}
#endif
	}

	if (isExpired(Second)) { handleExpiration(Second); }
	// User of second timer doesn't sleep on it.

}


void Timer::initTimers() {
	timerCallback[0] = nullptr;
	timerCallback[1] = nullptr;

	_expired[0] = false;
	_expired[1] = false;

	_isInUse[0] = false;
	_isInUse[1] = false;
}


/*
 * This should be kept short, any delay here adds to imprecision.
 * The timeout could be as little as 0 ticks (no reqt on parameter passed in.)
 * A timeout of 3 ticks @30uSec/tick takes 90uSec, which allows about 1440 instructions.
 */
void Timer::start(
		TimerIndex index,
		OSTime timeout,
		TimerCallback aTimeoutCallback){
	/*
	 * These assertions do not need be short to prevent sleep forever.
	 * But these assertions do affect the accuracy of the timeout
	 * (the more time we spend here, the later the real timeout will occur
	 * after the time for which the timeout was calculated.
	 */
	assert(timeout < MaxTimeout);
	assert(index < CountTimerInstances);
	// assert RTCx_IRQ enabled (enabled earlier for Counter, and stays enabled.

	// Not legal to start Timer already started and not timed out or canceled.
	if (isStarted(index)) {
		assert(false);
		return;	// No error result, must be tested with assertions enabled.
	}

	_isInUse[index] = true;
	timerCallback[index] = aTimeoutCallback;
	unexpire(index);

	configureCompareRegisterForTimer(index, timeout);
	// Timer may already be expired, interrupt generated, and callback called
}

bool Timer::isStarted(TimerIndex index) {
	return _isInUse[index];
}

void Timer::stop(TimerIndex index) {
	_isInUse[index] = false;
	// _expired[index] = false;
}


void Timer::expire(TimerIndex index) { _expired[index] = true; }
void Timer::unexpire(TimerIndex index) { _expired[index] = false; }
bool Timer::isExpired(TimerIndex index) { return _expired[index]; }

/*
 * This is called from RTCx_IRQHandler, with interrupts disabled.
 * The underlying CompareRegister has matched.
 * The underlying CompareRegister has already been disabled and event cleared.
 */
void Timer::handleExpiration(TimerIndex index) {

	/*
	 * Stop timer first, so callback can reuse timer.
	 */
	stop(index);

	/*
	 * Callback with reason: TimerExpired
	 * TODO not the correct reason for all Timers??
	 *
	 * This is still in interrupt context.
	 * Callback may generate more interrupts.
	 */
	timerCallback[index](SleepTimerCompare);	// call callback

	/*
	 * Assert signal sent (callback) and Timer is stopped.
	 * Assert signal won't be sent again.
	 * However, compare registers will still generate match events if counter cycles (4 minutes)
	 * and such events will wake any WEV.
	 */
}



void Timer::cancel(TimerIndex index){
	/*
	 * Legal to cancel Timer that has not been started.
	 * Legal to call from IRQ or main thread.
	 *
	 * Possible race: Timer IRQ may expire in the middle of this, so signal from Timer may happen anyway.
	 *
	 * We clear compare reg event, so it would not be set in a race.
	 */
	compareRegisters[index].disableInterruptAndClearEvent();

	stop(index);
	/*
	 * One-shot: assert:
	 * - compare interrupt is disabled.
	 * - compare event is cleared
	 *
	 * - callback is cleared
	 * - timer is unexpired
	 *
	 * Counter continues and compare reg still set, but it can't fire.
	 */
}
