#include <cassert>

#include "clockFacilitator.h"

#include "sleeper.h"

// From platform lib e.g. nRF5x or SiLAB
#include <drivers/oscillators/hfClock.h>
#include <drivers/nvic/nvic.h>
#ifdef SOFTDEVICE_PRESENT
// from libNRFDrivers
#include <lowFreqClockCoordinated.h>
#else
#include <drivers/oscillators/lowFreqClockRaw.h>
#endif




/*
 * Implementation notes:
 * We enable interrupt on LFCLOCKSTARTED, and leave it enabled.
 * The event comes and we disable interrupt but leave the event as a flag to show it is started.
 */


// include so it overrides default handler
// has not effect if MULTIPROTOCOL defined
// #include "../iRQHandlers/powerClockIRQHandler.cpp"
// April 2018 comment this out, make sure it still works


#ifndef SOFTDEVICE_PRESENT
namespace {

void lfClockStartedCallback() {
	Sleeper::setReasonForWake(ReasonForWake::LFClockStarted);
}

void hfClockStartedCallback() {
	/*
	 * Signal wake reason to sleep.
	 * Assert that no other interrupts can come and change reasonForWake.
	 * Otherwise, we should not set it directly, but prioritize it.
	 */
	Sleeper::setReasonForWake(ReasonForWake::HFClockStarted);
}

}
#endif

#ifndef SOFTDEVICE_PRESENT
void ClockFacilitator::startLongClockWithSleepUntilRunning(){

	/*
	 * In reverse order of dependencies.
	 */

	// Starting clocks with sleep requires IRQ enabled
	Nvic::enablePowerClockIRQ();

	// Convenient to register callback for HF clock here also.
	LowFreqClockRaw::registerCallbacks(lfClockStartedCallback, hfClockStartedCallback);

	LowFreqClockRaw::enableInterruptOnStarted();
	LowFreqClockRaw::configureXtalSource();
	// assert source is LFXO

	// We start LFXO.  LFRC starts anyway, first, but doesn't generate LFCLOCKSTARTED?
	LowFreqClockRaw::start();

	// Race: must sleep before LFCLOCKSTARTED event comes.  Takes .25 seconds for LFXO.  Takes .6mSec for LFRC.

	Sleeper::sleepUntilSpecificEvent(ReasonForWake::LFClockStarted);

	assert(LowFreqClockRaw::isRunning());

	// Enable Counter to start counting ticks of LFClock
	LongClock::start();

	// Disable interrupt since it may conflict with SD and other uses.
	// After CLOCKSTARTED interrupt, we don't expect (or can) receive other events/interrupts
	Nvic::disablePowerClockIRQ();

}


#else

void ClockFacilitator::startLongClockNoWaitUntilRunning() {
	/*
	 * LongClock requires LF clock running.
	 * LF clock module must be init.
	 */
	LowFreqClockCoordinated::init();
	LowFreqClockCoordinated::start();
	LongClock::start();
	// assert LongClock will begin ticking soon
}

#endif


bool ClockFacilitator::isLongClockRunning() {
	// delegate
	return LongClock::isOSClockRunning();
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
	 * !!! Not ensure HFXO is stably running.
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
	Sleeper::sleepUntilSpecificEvent(ReasonForWake::HFClockStarted);

	// assert ISR cleared event.

	HfCrystalClock::disableInterruptOnRunning();

	assert(HfCrystalClock::isRunning());
}


/*
 * Not require not already started.
 */
void ClockFacilitator::startHFXONoWait() {
	// Not enable interrupt
	HfCrystalClock::start();

	// Not ensure isRunning() since substantial delay e.g. 0.6mSec
}

void ClockFacilitator::stopHFXO() {
	HfCrystalClock::stop();
}
