
#pragma once

// platform lib e.g. nRF5x
//#include <drivers/gpioDriver.h>

#include <inttypes.h>

// GPIO index is not the same as a chip's pin number
typedef uint8_t GPIOIndex;	// [0..31]



typedef unsigned int LEDOrdinal;

/*
 * Manage array of LED's.
 *
 * This abstracts from an ordinal to a GPIO index.
 * I.E. you pass GPIO pins on init(), and thereafter an LED ordinal.
 * It also abstracts lit/dark from pin state (1/0) and electrical levels (source or sunk.)
 *
 * Singleton with class variables.  Not safe to define multiple instances.
 *
 * Toggle: LED's stay lit/dark until the next call,
 * so they indicate state of debugging by their transitions.
 *
 * !!! This is not thread safe: it doesn't test that pins remain configured as outputs.
 * Other code may interfere with hw registers.
 * Writing to a pin that other code has configured as input is harmless.
 *
 * You must call init() to guarantee pins are configured properly:
 * Valid sequence is: init(), toggleLEDs(), ...
 *
 * Managed pins should be dedicated, e.g. not physical pins shared with LFXO.
 */




class LEDService {
public:
	static void init(unsigned int count, bool arePinsSunk, GPIOIndex led1GPIO, GPIOIndex led2GPI0, GPIOIndex led3GPIO, GPIOIndex led4GPIO);
	static bool wasInit();

	/*
	 * For these, the ordinal must be in range [1..count] else asserts
	 */
	static void toggleLEDsInOrder();
	static void toggleLED(LEDOrdinal ordinal);
	static void switchLED(LEDOrdinal, bool state);

	/*
	 * FUTURE: ordinal out of range has no effect.
	 * no effect if ordinal is not mapped to a GPIO index at config time.
	 */
	static void trySwitchLED(LEDOrdinal, bool);
};
