Library API to a SoC (system on chip) having radio, cpu, and various peripherals.


Provides
-
     Platform indpendent API to radio and other common peripherals
     Simple services such as a mailbox
     Wrappers around certain board-level devices such as LED's.
     Exception handlers.
     Logging functions using Segger RTT
     
     
Used by SleepSyncAgent library.
Uses nRF5x library, a platform library for a particular brand (Nordic) SoC chips.
In other words, a layer between SleepSyncAgent (or any other radio app) and the platform.

The goal is to write another platform library for a different brand of radio SoC.
In other words, an experiment in portability.

Extracted from an earlier version of the nRF5x library.

For an example of the abstractions at this level:  Ensemble.  The Ensemble understands that to use the radio, the HF clock and the DCDC power supply need to be running.  A major concern is low-power: turning peripherals off and sleeping the mcu with only a low frequency clock/timer running.



Compile time flags that affect behavior
-

RESET_INSTEAD_OF_HALT whether to reset or halt when a fault occurs

LOGGING  Whether LOGGING is potent.  

Several build configurations implement logging using Segger RTT.  For these configurations, the RTTLogger::log() methods are potent.  In other build configurations, calls to those methods do nothing (conditionally compiled by LOGGING macro var.)  Since Segger RTT is platform independent, it is compiled in at this level.  However, the project currently uses Segger RTT files provided by one platform's SDK, which have been modified to include platform dependent include files.  Thus the build configuration is NOT platform independent, for now.

SOFTDEVICE_PRESENT	Whether library is compatible with Softdevice.  Same symbol as used in NRF_SDK

Multiprotocol
-

SOFTDEVICE_PRESENT usually means "sequential multiprotocol" i.e. alternate between using the Softdevice for Bluetooth protocol and implementing your own protocol.

Using Softdevice means the implementation is restricted: can't define some ISR's and thus can't conveniently use interrupts, but must poll or just wait for events.

Historically, the NRF51 builds were not SOFTDEVICE_PRESENT.  Now the NRF52 builds are always SOFTDEVICE_PRESENT.


Build configs
-

These should be named ...M0 and M4 since they are specific to the cpu architecture, not the radio chip.

    Debug52   nrf52xxx for use with NRF52DK debugger probe:  LOGGING enabled, debug level g3, no optimization 
    Debug51   nrf51xxx for use with or without debugger probe:  debug and no optimization

Build Configurations that log
-

Several build configurations implement logging using Segger RTT.  For these configurations, the RTTLogger::log() methods are potent.  In other build configurations, calls to those methods do nothing (conditionally compiled by LOGGING macro var.)

Segger RTT source code is in src/test/external and is not excluded from the build configuration. (In Eclipse, you see a slash through the icon when it IS excluded.  Click RightMouseButton on file and choose ResourceConfigurations>ExcludeFromBuild to see which build configs a source file is excluded from. )

Other settings:

    *Properties>C/C++ General>Paths and Symbols>Symbols* define LOGGING to make RTTLogger potent

Also, Segger RTT source depends on Nordic SDK app_util_platform.cpp for critical section functions.  Currently not built into the library, but built into calling apps.  Also, so .h files are found, app build config has include paths to "${NRF_SDK}/external/segger_rtt", "${NRF_SDK}/components/libraries/util", and "${NRF_SDK}/components/drivers_nrf/nrf_soc_nosd" .  FIXME, move these to the library build configs.

Boards
-
Some services (e.g. ledLogger) depend on the board configuration (what pins are configured as digital out to LEDs, and what revision of the chip is on the board.)

The service allows runtime configuration from board configurations in higher layers.


Exception Handlers
-
Also called faults (not to be confused with C++ exceptions.)

Two kinds of faults handled:

    hw faults e.g. HardFault_Handler
    sw faults e.g. the handler called by the assert() macro

The library provides generic routines for fault handlers.
They can write to flash to record the faults for later analysis.
A calling app should override certain default handlers and call the generic routines.  
Default handlers enter an infinite loop, without entering low power mode.  

The generic default handlers:

    for production, reset instead of infinite loop
    for debug using a low-reserve power supply, reduce power before sleeping
    
For production, soft reset is preferable to infinite loop so the system might recover, without human intervention to hard reset.
 
For debug, reducing power and sleeping keeps the system in a debuggable state (otherwise an infinite loop quickly causes brown out.)
 
(Future: for debug, the handlers should save system registers from the program context in variables i.e. save the top level of the stack trace.)

Brownout warning
-
The library also provides routines for detecting and handling brownout warning.
The routines can write to flash a record that brownout occurred.
A brownout warning is not considered a fault, only a warning of impending real BOR fault
(when the cpu stops executing and holds in the reset condition because power is too low.)

