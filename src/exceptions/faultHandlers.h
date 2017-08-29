#pragma once


/*
 * Generic fault handlers.
 *
 * This is specialized for this ultra-low power app:
 * - in production, soft reset
 * - in debugging, turn off devices (radio and RTC) and sleep in a loop.
 *
 * Your app can redefine the default handlers to call these generic handlers.
 * The default handlers are weakly defined and 'called' from interrupt vectors (Nordic: in gcc_startup.S)
 *
 * You SHOULD redefine the default handlers.
 * Default handlers are inadequate, simply looping.
 * Looping will lead high current consumption, to brownout, then power recovery, and reset.
 * In that scenario, the app may appear to keep functioning, but have gone through a reset.
 *
 * For that scenario, where brownout reset occurs,
 * these handlers leave a trace in flash.
 * Thus you can put the system on a debugger probe to read flash to determine what happened.
 *
 * If you define NDEBUG, assertions will be off and the only fault should be HardFault.
 *
 * These are generic and should not depend on the target i.e. Nordic.
 * I.E. the implementation should call other modules, not Nordic dependent drivers.
 */

extern "C" {

__attribute__((noreturn))
void resetOrHalt();

__attribute__((noreturn))
void sleepForeverInLowPower();

#ifdef NOT_USED
void genericNMIHandler();    // Certain peripherals or SW. Non-maskable, only preempted by reset
void genericSVCHandler();    // call to OS: SVC instruction executed
void genericPendSVHandler(); // OS Context switching
void genericSysTickHandler(); // OS clock
#endif


void HardFault_Handler();


// M0 bus faults and other hw faults
void genericHardFaultHandler();


/*
 * assertion failed: assert() calls abort() and then _exit()
 *
 * The _exit() defined in newlib nano infinite loops.
 * For an embedded app with nanopower, better to indicate fault and then sleep.
 *
 * You must override _exit() defined in newlib nano to call this.
 * Apparently not easy to override from a library.
 */
void genericExitHandler();

/*
 * Assertion failed.
 * Write subset of details to flash
 */
void genericAssertionFaultHandler(const char* functionName, int lineNumber);


/*
 * Did fault occur sometime since UICR was cleared?
 */
bool wasHardFault();
bool wasAssertionFault();


// M4 also defines other faults, subclasses, having their own vectors
/*
MemoryManagement_Handler	memory protection violation
BusFault_Handler	odd or undefined address
UsageFault_Handler undefined opcode, etc.
*/

/*
 * Handler for:
 * - NRF_SDK library errors.
 * - assert() macro (Nordic definition of assert() calls this?)
 *
 * The default handler does: app_error_save_and_stop(id, pc, info);
 */
//void app_error_fault_handler(uint32_t id, uint32_t pc, uint32_t info);


}	// extern C
