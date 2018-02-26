
#include "ledFlasherTask.h"


#include "ledFlasher.h"	// amountInTicks
#include "../clock/eventTimer.h"

// lib nRF5x
#include "drivers/pinTask.h"
#include "drivers/eventToTaskSignal.h"

// lib nRF5x
#include <drivers/clock/counter.h>
//#include <drivers/clock/compareRegister.h>

#include <cassert>


void LEDFlasherTask::init(uint32_t pin ) {


	// EventTimer not need init but LFClock and RTC counter must be ticking
	assert(Counter::isTicking());

	// PinTask must be init
	PinTask::configureTaskOnPinToToggleInitiallyHigh( pin );
	PinTask::enableTask();

	// Now, we are only users of PPI
	/*
	 * Connect signal from EventTimer to task
	 */
	uint32_t* taskAddress = PinTask::getTaskRegisterAddress();
	uint32_t* eventAddress = EventTimer::getEventRegisterAddress();

	EventToTaskSignal::connect(eventAddress,taskAddress);
}


void LEDFlasherTask::flashLEDByAmount(unsigned int amount) {
	/*
	 * assert initialized:
	 * - EventTimer
	 * - PPI
	 * - PinTask
	 */

	// Return if already flashing.
	// TODO is there any way to do this?

	// toggle LED on
	PinTask::startTask();

	// start timer that will generate event connected via PPI to task which will toggle off
	EventTimer::start(LEDFlasher::amountInTicks(amount));
}
