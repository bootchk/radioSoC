
#include <cassert>

// TODO move to nRF5x
//#include "nrf.h"
//#include "nrf_clock.h"

#include "powerAssertions.h"

#include "../ensemble/ensemble.h"

// platform lib e.g. nRF5x
#include <drivers/powerComparator.h>
#include <drivers/flashController.h>
#include <drivers/mcu.h>


void assertUnusedOff();	// local
void assertNeverUsedDevicesOff();



/*
 * For debugging, assert conditions proper to allow ultra low power.
 * The nrf chip will power down peripherals and clocks not used to achieve low power.
 *
 * Do not call when radio is being used: HF XTAL source and radio enabled.
 *
 * Some coded w/o using drivers, i.e. not portable
 *
 * PowerComparator??
 * TODO GPIO's configured disconnected
 * TODO no interrupts/events pending that would prevent sleep
 */

/*
 * Peripherals we never use at all.
 */
void assertNeverUsedDevicesOff() {

#ifdef FUTURE
	This was all working but since radioSoC was extracted, needs to be made platform independent
#ifdef NRF52
	// nrf52 FPU is disabled.
	assert(SCB->CPACR == 0);

	// FPU exception not set, errata 87
	#define FPU_EXCEPTION_MASK 0x0000009F
	assert(! (__get_FPSCR()  & FPU_EXCEPTION_MASK) );

	// FPU interrupt not pending
	assert( ! NVIC_GetPendingIRQ(FPU_IRQn));

	// MWU disabled
	assert(NRF_MWU->REGIONEN == 0);

	// Not in debug mode
	// ??? not defined in SDK: DHCSR
	// Only use this if solar power
    //assert (DWT->CYCCNT == 0);
#endif


	TODO move to nRF5x
	// Peripherals in reset condition

	// WDT not running
	assert(NRF_WDT->RUNSTATUS == 0);


	// TODO Not exist a register to detect power sub-mode (LOWPOWER or CONSTANT_LATENCY)
	// and Nordic docs say it defaults to low-power, but we could trigger the task here
	// NRF_POWER->TASKS_LOWPWR = 1;

	// GPIO's inputs and disconnected
	// TODO only the ones we don't use

	// Pending flags prevent sleep even if interrupts disabled
	//NVIC_ClearPendingIRQ(FPU_IRQn);
	assert( ! NVIC_GetPendingIRQ(RADIO_IRQn));
	assert( ! NVIC_GetPendingIRQ(RTC1_IRQn));
	assert( ! NVIC_GetPendingIRQ(POWER_CLOCK_IRQn));
	assert( ! NVIC_GetPendingIRQ(ADC_IRQn));

	// TODO other, unused IRQ such as WDT, MemCU

	// No exceptions mcu
	assert( ! (__get_IPSR() & IPSR_ISR_Msk) );


	// Not reset for unexpected reason
	// This doesn't work in debug because the reason will be DIF
	// assert( NRF_POWER->RESETREAS == 0 );
#endif
}


#ifdef NDEBUG
void assertUltraLowPower() { return; }
void assertRadioPower() { return; }
#else

/*
 * All peripherals except RTC and LFXO off.
 * Including peripherals we use, but should be inactive at time of call: radio and its HFXO.
 */
void assertUltraLowPower() {

	assertNeverUsedDevicesOff();

	assert(Ensemble::isLowPower());

	/*
	 * Not: assert( PowerComparator::isDisabled());
	 * PowerComparator stays enabled for brownout detect.
	 *
	 * Not: assert( ADC::isDisabled());
	 * The ADC stays enabled but uses no power when not converting.
	 */

	assert( FlashController::isDisabled());

	// DCDC power regulator disabled
	// TODO from nRF5x
	// assert(NRF_POWER->DCDCEN == 0);

	// Only effective for nrf52
	// isDebugMode() doesn't seem to work:  assert(!MCU::isDebugMode());
}

void assertRadioPower() {

	assertNeverUsedDevicesOff();
}

#endif

