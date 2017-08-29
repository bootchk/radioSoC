
#pragma once

#include <inttypes.h>


// Requires long long to hold it
typedef uint64_t DeviceID ;


class System {
public:

	// 6 bytes, 48 bits
	static const DeviceID MaxDeviceID = 0xfFfFfFfFfFfF;


	/*
	 * A substring (6 bytes of) full MAC ID (8 bytes) of radio as provided by platform.
	 * Embedded in payload.
	 * Not the same as protocol address.
	 */
	static DeviceID ID();

	/*
	 * For debugging, only the least significant byte of the above.
	 * Typically it is unique enough for small swarms of radios.
	 */
	static uint8_t shortID();

};
