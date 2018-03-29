

#include <inttypes.h>



//#include "../drivers/mcu.h"
//#include "../drivers/nvic.h"
#include <drivers/mcu.h>
#include <drivers/nvic/nvic.h>

#include "../services/customFlash.h"

#include "faultHandlers.h"

// Whether to soft reset versus low-power infinite loop
// Usually defined in build configuration, -D flag to compiler
// #define RESET_INSTEAD_OF_HALT 1


/*
 * get_MSP is defined in components/toolchain/cmsis/include/cmsis_gcc.h
 * but it requires other include files and is platform specific.
 * This was copied from there.
 * <core_cm0.h>	// __STATIC_INLINE
 * <cmsis_gcc.h>    // get_MSP
 * or <nrf_power.h>	// get_MSP
 */
/**
  \brief   Get Main Stack Pointer
  \details Returns the current value of the Main Stack Pointer (MSP).
  \return               MSP Register value
 */
__attribute__( ( always_inline ) ) static inline uint32_t __get_MSP(void)
{
  register uint32_t result;

  __asm volatile ("MRS %0, msp\n" : "=r" (result) );
  return(result);
}



// TODO should be a callback to app which knows what peripherals are used
// #include "../radio/radio.h"
namespace {

/*
 * Power off any used devices (specific to app).
 * So that the mcu might not brownout, but the app is halted (non-functional.)
 *
 * If instead, you brownout, the mcu resets and might appear to continue functioning,
 * with no external indication that a fault occurred.
 *
 * This is application specific, for the peripherals I am using.
 */
void powerOffPeripherals() {

	// TODO turn off RTC
	// LongClock::stop();

	// TODO turn off GPIO's (e.g. LED)

	// Make radio not transceiving (chip will power it off.)
	// Radio::abortUse();

	// Also power off radio's dependencies (HFXO)
	// Radio::shutdownDependencies();
}



} // namespace





extern "C" {

/*
 * Conditionally compile:
 * - Release mode: reset and keep trying to function
 * - Debug mode: sleep forever in low power
 *
 * Even if reset, an exception might be recorded in UICR
 */
__attribute__((noreturn))
void resetOrHalt() {

#ifdef RESET_INSTEAD_OF_HALT
	// Reset (boot and start over.)
    Nvic::softResetSystem();
#else
    sleepForeverInLowPower();
#endif // DEBUG
}

/*
 * A fault has occurred.
 * Try to stay out of brownout.
 *
 * This is only useful in that you can see that the app has stopped
 * (instead of resetting and appearing to run normally.)
 * It might be more useful if you can attach debugger and break into the running program.
 * Debug mode takes 1-4mA so likely that solar power is not enough to support
 */
__attribute__((noreturn))
void sleepForeverInLowPower() {
	// Most important to try record what happened.
	// The caller should do that before calling this.

	// Disable all further interrupts from peripherals that might be concurrently operating.
	MCU::disableIRQ();

	// Optionally reduce power so brownout doesn't compound the fault
	powerOffPeripherals();

	// Since there are no peripherals and no timer, this will sleep in very low power
	// The only thing that can wake it is an interrupt signal on a pin (not an internal interrupt from device on SoC.)
	while(true) {
		MCU::sleepUntilEvent();
	}
}

#if USE_SOFT_DEVICE
/*
 * Handlers for exceptions/faults.
 * These override the defaults, which are infinite loops and quickly brownout a low-power supply.
 */


void app_error_fault_handler(uint32_t id, uint32_t pc, uint32_t info)
{
	(void) id;	// unused params
	(void) pc;
	(void) info;

	resetOrHalt();
}
#endif


/*
 * Handlers for hardware faults.
 *
 * nRF51 subset (nRF52 has more)
 *
 * !!! Might not expect most of these; only HardFault can occur regardless of the application.
 * The rest might never occur depending on the design of the app:
 * - not using an OS that would generate SVC
 * - not using SysTick
 * - not using reset pin that would generate NMI
 */
/*
 * The "unused" attribute simply suppresses a compiler warning.
 * It does not guarantee that a definition overrides a weak ASM definition?
 */


// HardFault capture PC


#ifdef NOT_USED
void genericNMIHandler()  { resetOrHalt(); }
void genericSVCHandler(void) { resetOrHalt(); }
void genericPendSVHandler(void) { resetOrHalt(); }
void genericSysTickHandler(void) { resetOrHalt(); }
#endif

/*
 * Write PC to distinct word.
 *
 * !!! The calling app must reference some symbol in this file to insure
 * that this definition overwrites the weak def earlier at link time.
 */
__attribute__ ((interrupt ("IRQ")))
void
HardFault_Handler() {
	// Get fault address.
	// Specific to case where MSP is used (w/o RTOS)
	uint32_t* stackPointer = (uint32_t*) __get_MSP();
	uint32_t faultAddress = stackPointer[12]; // HW pushed PC onto stack 6 words into stack frame

	// Only write it once, for first hard fault
	CustomFlash::tryWriteIntAtIndex(HardFaultPCIndex, faultAddress);
	resetOrHalt();
}


/*
 * Catch all handler.
 * Many faultHandlers or other code may call this.
 * In that case, the set flag does not distinguish between the callers.
 */
__attribute__((noreturn))
void genericExitHandler(void) {
	CustomFlash::writeZeroAtIndex(ExitFlagIndex);
	resetOrHalt();
}


void genericAssertionFaultHandler(const char* locationText, int lineNumber){
	// Write line no. in one word, and text in other locations in UICR
	CustomFlash::tryWriteIntAtIndex(LineNumberIndex, lineNumber);
	CustomFlash::copyStringPrefixToFlash(locationText);
	resetOrHalt();
}

bool wasHardFault() { return CustomFlash::isWrittenAtIndex(HardFaultPCIndex); }
bool wasAssertionFault() { return CustomFlash::isWrittenAtIndex(ExitFlagIndex); }



} // extern C
