Library API to a SoC (system on chip) having radio, cpu, and various peripherals.

Used by SleepSyncAgent library.
Uses nRF5x library, a platform library for a particular brand (Nordic) SoC chips.
In other words, a layer between SleepSyncAgent (or any other radio app) and the platform.

The goal is to write another platform library for a different brand of radio SoC.
In other words, an experiment in portability.

Extracted from an earlier version of the nRF5x library.

The layering is not one-way: there remains a few calls upward from nRF5x into radioSoC, which instead should be registered callbacks.

For an example of the abstractions at this level:  Ensemble.  The Ensemble understands that to use the radio, the HF clock and the DCDC power supply need to be running.  A major concern is low-power: turning peripherals off and sleeping the mcu with only a low frequency clock/timer running.