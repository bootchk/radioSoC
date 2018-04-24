
#include "taskTimer.h"

//#include <clock/timer.h>

#include "../services/logger.h"
#include "longClock.h"

// platform lib nRF5x
#include <drivers/clock/compareRegister.h>
#include <drivers/clock/compareRegArray.h>

#ifdef SOFTDEVICE_PRESENT
   // from libNRFDrivers
   #include <nvicCoordinated.h>
#else
   // from nRF5x
   #include <drivers/nvic/nvicRaw.h>
#endif


#include <cassert>


// TODO eliminate wrapper

namespace {
/*
 * Routine to call in ISR context (i.e. a Task)
 */
Task aTask = nullptr;

/*
 * TaskTimer can expire without underlying compare register events.
 */
bool _isExpired = false;

bool _isInUse = false;

}	// namespace






void TaskTimer::timerISR() {

	/*
	 * Called from RTCx_IRQ.
	 * That may be called for counter overflow.
	 * This should only be called when interrupt was pended or CompareRegister matched.
	 */

	/*
	 * Source events are "compare register matched counter"
	 * First just determine which Timers are expired because their CompareRegister fired.
	 * (They may also be expired because their duration was too short, without a CompareRegister event.)
	 */
	if ( compareRegisters[0].isEvent() ) {
		compareRegisters[0].disableInterruptAndClearEvent();	//  early
		_isExpired = true;
	}

	/*
	 * Second handle all expired Timers.
	 * Some may have expired without their CompareRegister firing (for short timeout, via an interrupt pended i.e. forced.)
	 */
	if (_isExpired) {
		handleExpiration();
	}
}


/*
 * Called from RTCx_IRQHandler and timerISR, in ISR context (interrupts at same priority prevented.)
 * When the underlying CompareRegister has matched, it has already been interrupt disabled and event cleared.
 */
void TaskTimer::handleExpiration() {

	/*
	 * Mark not in use, so called task can reuse timer.
	 */
	_isInUse = false;

	/*
	 * Callback, still in interrupt context.
	 * Callback may generate more interrupts.
	 */
	aTask();

	/*
	 * Assert Task completed.
	 * Task may be reusing TaskTimer already.
	 *
	 * Compare registers will still generate match events if counter cycles (4 minutes)
	 * and such events will wake any WEV.
	 * But unless the Task is using TaskTimer again,
	 * no interrupts are enabled for match events.
	 */
}


void TaskTimer::schedule(Task task, OSTime duration) {
	// Not legal to start Timer already started and not timed out or canceled.
	// Not legal to start twice, catch before destroying old task.
	assert(not _isInUse);

	aTask = task;

	// Usually name of task is logged just ahead of this
	RTTLogger::log(":"); RTTLogger::log(duration);

#ifdef OLD
	we used Timer, with a wrapper timerCallback
	Timer::start(
			First,
			duration,
			timerCallback);
#endif

	/*
	 * These assertions do not need be short to prevent sleep forever.
	 * But these assertions do affect the accuracy of the timeout
	 * (the more time we spend here, the later the real timeout will occur
	 * after the time for which the timeout was calculated.
	 */
	//assert(duration < MaxTimeout);
	// assert RTCx_IRQ enabled (enabled earlier for Counter, and stays enabled.

	_isExpired = false;
	_isInUse = true;

	configureCompareRegisterForTimer(0, duration);
	// Timer may already be expired, interrupt generated, and callback called
}




/*
 * A Timer is built upon a CompareRegister.
 * Since a CompareRegister has limitations, do more here to accommodate limitations.
 * This is a concern of the Timer, not of the CompareRegister.
 *
 * Should be no references to platform here.
 */
void TaskTimer::configureCompareRegisterForTimer(TaskTimerIndex index, OSTime timeout){
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
		// Mark timer expired already (the small duration is elapsed already.)
		_isExpired = true;
		/*
		 * Pend interrupt.
		 * The RTCx_IRQ is always enabled, this might generate immediate jump to ISR.
		 * When caller is a task, in ISR context, the interrupt will process when this task completes.
		 * ISR will see the expired timer, and handle it, even though no event from compare register.
		 */
#ifdef SOFTDEVICE_PRESENT
		NvicCoordinated::pendLFTimerInterrupt();
#else
		NvicRaw::pendLFTimerInterrupt();
#endif
	}
	else { // Guaranteed that CompareRegister will generate event and interrupt.
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


