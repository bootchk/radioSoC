
#include "../clock/longClock.h"
#include "../clock/taskTimer.h"



/*
 * One handler for many interrupt sources (overflow, and compare regs)
 * Might exist many instances of same source (many Timers using many CompareRegisters.)
 * Many sources can be pending, so handle them all.
 *
 * !!! The interrupt can come with no events, since SW can pend the interrupt in the NVIC
 *
 * !!! One Timer expired may callback more than one user of Timer!
 * E.G. user of SleepTimer may be called back when other Timer expires.
 *
 * Overrides weak default handler defined by gcc_startup_nrf52.c.
 * Binding must be "C" to override default handler.
 */

// From platform lib e.g. nRF5x or SiLAB
// Choose RTC instance
#include <drivers/hwConfig.h>



extern "C" {


void RTC2_IRQHandler();
void RTC1_IRQHandler();


// Hook to a RTC interrupts
__attribute__ ((interrupt ("IRQ")))
void
#if LFTimerUseRTC2 == 1
   RTC2_IRQHandler(void)
#elif LFTimerUseRTC1 == 1
    RTC1_IRQHandler(void)
#else
#error "no RTCx_IRQ"
#endif
{


	// Source event: Counter overflow
	LongClock::longClockISR();

	// Source event: CompareRegister match
	TaskTimer::timerISR();

	/*
	 * If we cleared any events, enough time (4 clock cycles) has elapsed
	 * so that interrupt will not be requested immediately after we return from interrupt.
	 * See "Peripheral Interface" in Nordic Product Spec.
	 */
	/*
	 * If any events have triggered after we checked them,
	 * they will still trigger an interrupt and this handler will be called again.
	 */
}


}	// extern "C"
