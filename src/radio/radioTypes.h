
#pragma once

#include <inttypes.h>

/*
 * Types common to radio and radio use cases
 */


/*
 * Transmit power relative to one milliwatt, units are dBm.
 */
enum class TransmitPowerdBm : int8_t {
	Plus4 = 0x04,
	// Plus3 = 0x03,	// Not available on nrf51
	Plus0 = 0x00,
    Minus4 = -4,
	Minus8 = -8,
	Minus12 = -12,
	Minus16 = -16,
	Minus20 = -20,
	Minus40 = -40
};

/*
Minus4 = 0xFC,
	Minus8 = 0xF8,
	Minus12 = 0xF4,
	Minux16 = 0xF0,
	Minus20 = 0xEC,
	Minus40 = 0xD8
*/
