#include <cassert>

#include "clockFacilitator.h"

#include "sleeper.h"

// From platform lib e.g. nRF5x or SiLAB
#include <drivers/lowFrequencyClock.h>
#include <drivers/hfClock.h>
#include <drivers/nvic.h>



/*
 * Implementation notes:
 * We enable interrupt on LFCLOCKSTARTED, and leave it enabled.
 * The event comes and we disable interrupt but leave the event as a flag to show it is started.
 */


// include so it overrides default handler
#include "../iRQHandlers/powerClockIRQHandler.cpp"


void ClockFacilitator::startLongClockWithSleepUntilRunning(){

	/*
	 * In reverse order of dependencies.
	 */

	// Starting clocks with sleep requires IRQ enabled
	Nvic::enablePowerClockIRQ();

	LowFrequencyClock::enableInterruptOnStarted();
	LowFrequencyClock::configureXtalSource();
	// assert source is LFXO

	// We start LFXO.  LFRC starts anyway, first, but doesn't generate LFCLOCKSTARTED?
	LowFrequencyClock::start();

	// Race: must sleep before LFCLOCKSTARTED event comes.  Takes .25 seconds for LFXO.  Takes .6mSec for LFRC.

	Sleeper::sleepUntilEvent(ReasonForWake::LFClockStarted);

	assert(LowFrequencyClock::isRunning());

	// Finally, enable Counter to start counting ticks of LFClock
	LongClock::start();

}


void ClockFacilitator::startHFClockWithSleepConstantExpectedDelay(OSTime delay){
	assert( !HfCrystalClock::isRunning() );

	// Don't care whether event is set since not using interrupt on event
	// assert(!HfCrystalClock::isStartedEvent());

	// We don't want interrupt
	assert(!HfCrystalClock::isInterruptEnabledForRunning());

	Sleeper::clearReasonForWake();
	HfCrystalClock::start();

	// cruft
	// TODO depends on SyncSleeper.  Maybe move SyncSleeper to nRF5x??? or ClockFacilitator up?
	//SyncSleeper::sleepUntilTimeout(delay);

	// Blocking
	Sleeper::sleepDuration(delay);

	/*
	 * !!! Not unsure HFXO is stably running.
	 * We waited a constant time that is expected, but not guaranteed for HFXO to be running.
	 * The constant time might depend on measurements of actual board implementations of crystal network.
	 */
}



void ClockFacilitator::startHFXOAndSleepUntilRunning() {
	/*
	 * Illegal to call if already running.
	 * In that case, there might not be an event or interrupt to wake,
	 * or the interrupt could occur quickly after we start() but before we sleep (WFI)
	 */
	assert( !HfCrystalClock::isRunning() );
	assert(!HfCrystalClock::isStartedEvent());

	// Interrupt must be enabled because we sleep until interrupt
	HfCrystalClock::enableInterruptOnRunning();

	/*
	 * We should not be sleeping, but other low-priority reasons such as BrownoutWarning etc.
	 * could be set and lost by this clear.
	 */
	Sleeper::clearReasonForWake();
	HfCrystalClock::start();
	/*
	 * Event may have come already, but there is not a race to clear reason.
	 * sleepUntilEvent does NOT clearReasonForWake.
	 */

	// Blocking
	Sleeper::sleepUntilEvent(ReasonForWake::HFClockStarted);

	// assert ISR cleared event.

	HfCrystalClock::disableInterruptOnRunning();

	assert(HfCrystalClock::isRunning());
}

