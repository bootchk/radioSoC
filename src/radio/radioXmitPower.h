
#pragma once

#include <inttypes.h>


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
	Minus40 = -40,
	Invalid
};

/*
Minus4 = 0xFC,
	Minus8 = 0xF8,
	Minus12 = 0xF4,
	Minux16 = 0xF0,
	Minus20 = 0xEC,
	Minus40 = 0xD8
*/


/*
 * Conversions between types used by device and type TransmitPowerdBm
 */
class XmitPower {
public:
	// Is an OTA value valid?
	static bool isValidXmitPower(int8_t aValue);

	// Convert OTA value to enum
	static TransmitPowerdBm xmitPowerFromRaw(int8_t rawXmitPower);

	static int8_t rawXmitPower(TransmitPowerdBm xmitPower);

	static const char * repr(TransmitPowerdBm xmitPower);
};
