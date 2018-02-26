
#include "eventTimer.h"

// Implementation
#include "longClock.h"

// platform lib
//#include <drivers/nvic/nvic.h>
#include <drivers/clock/compareRegister.h>
#include <drivers/clock/compareRegArray.h>



/*
 * Implemented using one CompareRegister of RTC (index 2)
 */

#define EVENT_TIMER_INDEX 2


namespace {

unsigned int compareValueForTimeout(OSTime timeout) {

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
	return beforeCounter + timeout;
}

}



void EventTimer::start(OSTime timeout) {
	// Disable in case compare reg is already near match
	compareRegisters[EVENT_TIMER_INDEX].disableEventSignal();

	/*
	 * !!! Not ensuring that newCounterValue is not too near current value
	 * i.e. not ensuring timely event
	 */
	compareRegisters[EVENT_TIMER_INDEX].set(compareValueForTimeout(timeout));

	compareRegisters[EVENT_TIMER_INDEX].enableEventSignal();
}



void EventTimer::stop() {
	compareRegisters[EVENT_TIMER_INDEX].disableEventSignal();
	// Not change compare register value
}

uint32_t* EventTimer::getEventRegisterAddress() {
	return compareRegisters[EVENT_TIMER_INDEX].getEventRegisterAddress();
}
