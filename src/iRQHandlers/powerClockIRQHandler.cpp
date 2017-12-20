
/*
 * This uses POWERCLOCK device, restricted by Softdevice
 */


#ifndef MULTIPROTOCOL


// platform lib
#include <drivers/lowFreqClockRaw.h>
#include <drivers/powerComparator.h>


// C so overrides weak handler, without C++ name mangling
extern "C" {

// Compiler warning if don't declare
void POWER_CLOCK_IRQHandler();


/*
 * ISR for Power and Clock devices.
 * POFCON and clocks (LF, HF) use same IRQ vector.
 *
 * Two functions:
 * - interrupt on clocks running
 * - interrupt on POFCON detect brownout
 */
__attribute__ ((interrupt ("IRQ")))
void
POWER_CLOCK_IRQHandler() {
	/*
	 * Typically, interrupts enabled for events:
	 * - EVENT_HFCLKSTARTED
	 * - EVENT_LFCLKSTARTED
	 * - EVENTS_POFWARN (Power Fail Warning)
	 */

#ifdef NOT_USED
	// Get fault address.
	// Specific to case where MSP is used (w/o RTOS)
	uint32_t* stackPointer = (uint32_t*) __get_MSP();
	uint32_t faultAddress = stackPointer[12]; // HW pushed PC onto stack 6 words into stack frame
#endif

	// XXX if ! ( EVENT_LFCLKSTARTED || EVENT_HFCLKSTARTED || EVENTS_POFWARN ) assert programming bug
	// an event that we don't handle properly

	/*
	 * Each of powerISR and clockISR may set reasonForWake,
	 * so they must be aware of each other, or prioritize reasonForWake.
	 */
	PowerComparator::powerISR();

#ifndef MULTIPROTOCOL
	LowFreqClockRaw::clockISR();
#else
	LowFreqClockCoordinated::clockISR();
#endif
}




}	// C

#endif


