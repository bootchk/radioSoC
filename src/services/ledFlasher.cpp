
#include <cassert>

#include "ledFlasher.h"

#include "../modules/ledService.h"
// implementation uses pure class LongClock already included by header
#include "../clock/timer.h"



namespace {

/*
 * Callback from timer, keep it short.
 * The timer interrupt wakes the mcu if was sleeping.
 */
void ledOffCallback(TimerInterruptReason reason) {
	// We don't callback for Overflow or other timers
	assert(reason == SleepTimerCompare);

	// TODO we should turn off led that is on, here assume it is first LED, ordinal 1
	LEDService::switchLED(1, false);
	// Timer cancels itself
}

} // namespace




void LEDFlasher::flashLEDMinimumVisible(unsigned int ordinal) {
	flashLEDByAmount(ordinal, MinFlashAmount);
}

void LEDFlasher::flashLEDByAmount(unsigned int ordinal, unsigned int amount){
	// assert LEDService initialized
	// assert TimerService initialized

	// Check range
	assert(amount>=MinFlashAmount);
	// Clamp to max
	if (amount > MaxFlashAmount )  amount = MaxFlashAmount;


	// Return if already flashing.
	if (Timer::isStarted(Second)) {	return; }


	LEDService::switchLED(ordinal, true);

	// Calculate timeout in units ticks from amount units
	OSTime timeout = amount * TicksPerFlashAmount;

	// start timer to turn LED off
	Timer::start(
			Second,
			timeout,
			ledOffCallback
			);
}
