Library API to a SoC (system on chip) having radio, cpu, and various peripherals.

Used by SleepSyncAgent library.
Uses nRF5x library, a platform library for a particular brand (Nordic) SoC chips.
In other words, a layer between SleepSyncAgent (or any other radio app) and the platform.

The goal is to write another platform library for a different brand of radio SoC.
In other words, an experiment in portability.

Extracted from an earlier version of the nRF5x library.

The layering is not one-way: there remains a few calls upward from nRF5x into radioSoC, which instead should be registered callbacks.

For an example of the abstractions at this level:  Ensemble.  The Ensemble understands that to use the radio, the HF clock and the DCDC power supply need to be running.  A major concern is low-power: turning peripherals off and sleeping the mcu with only a low frequency clock/timer running.



Compile time flags that affect behavior:

RESET_INSTEAD_OF_HALT whether to reset or halt when a fault occurs

LOGGING  Whether LOGGING is potent.  

Several build configurations implement logging using Segger RTT.  For these configurations, the RTTLogger::log() methods are potent.  In other build configurations, calls to those methods do nothing (conditionally compiled by LOGGING macro var.)  Since Segger RTT is platform independent, it is compiled in at this level.  However, the project currently uses Segger RTT files provided by one platform's SDK, which have been modified to include platform dependent include files.  Thus the build configuration is NOT platform independent, for now.


Build configs

Debug52   nrf52xxx for use with NRF52DK debugger probe:  LOGGING enabled, debug level g3, no optimization 
Debug51   nrf51xxx for use with or without debugger probe:  debug and no optimization