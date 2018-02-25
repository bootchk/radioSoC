
#include <cassert>

#include "ledFlasher.h"

#include "../modules/ledService.h"
// implementation uses pure class LongClock already included by header
#include "../clock/timer.h"




namespace {

// for scheduled flash, read by ledOnCallback
unsigned int _amount;
unsigned int _ordinal;



/*
 * Callbacks from timer, keep them short.
 * The timer interrupt wakes the mcu if was sleeping.
 */


void ledOffCallback(TimerInterruptReason reason) {
	// We don't callback for Overflow or other timers
	assert(reason == SleepTimerCompare);

	// TODO we should turn off led that is on, here assume it is first LED, ordinal 1
	LEDService::switchLED(1, false);
	// Timer cancels itself
}

/*
 * Called at time to toggle LED on.
 */
void ledOnCallback(TimerInterruptReason reason) {
	// We don't callback for Overflow or other timers
	assert(reason == SleepTimerCompare);

	// Timer just went off, it is not in use

	LEDFlasher::flashLEDByAmount(_ordinal, _amount);
}


/*
 * LED has exclusive use of Second timer
 */
void startTimerToTurnLEDOff(OSTime timeout) {
	Timer::start(
			Second,
			timeout,
			ledOffCallback
	);
}
void startTimerToTurnLEDOn(OSTime timeout) {
	Timer::start(
			Second,
			timeout,
			ledOnCallback
	);
}


} // namespace




void LEDFlasher::flashLEDMinimumVisible(unsigned int ordinal) {
	flashLEDByAmount(ordinal, MinFlashAmount);
}

void LEDFlasher::flashLEDByAmount(unsigned int ordinal, unsigned int amount){
	// assert LEDService initialized
	// assert TimerService initialized


	// Return if already flashing.
	if (Timer::isStarted(Second)) {	return; }


	LEDService::switchLED(ordinal, true);

	startTimerToTurnLEDOff(amountInTicks(amount));
}


void LEDFlasher::scheduleFlashLEDByAmount(unsigned int ordinal, unsigned int amount, unsigned int ticks){

	// Remember stuff
	_ordinal = ordinal;
	_amount = amount;

	// We check amount later

	startTimerToTurnLEDOn(ticks);
}




unsigned int LEDFlasher::amountInTicks(unsigned int amountInMinFlashes) {
	unsigned int candidateAmount = amountInMinFlashes;

	// Check range
	assert(amountInMinFlashes>=MinFlashAmount);
	// Clamp to max
	if (amountInMinFlashes > MaxFlashAmount )  candidateAmount = MaxFlashAmount;

	// Calculate timeout in units ticks from amount units
	return candidateAmount * TicksPerFlashAmount;
}

