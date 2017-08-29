
#include <cassert>
#include "system.h"

// platform lib e.g. nRF5x
#include <drivers/uniqueID.h>



/*
 * A substring (6 bytes of) full MAC ID (8 bytes) of radio as provided by platform.
 * Embedded in payload.
 * Not the same as protocol address.
 */
DeviceID System::ID() {
	DeviceID result;

	result = deviceID();
	// result upper bytes are all ones

	// Mask off upper bytes, to match over-the-air length of 6 bytes.
	result = result & MaxDeviceID;

	assert(result <= MaxDeviceID);
	return result;
}

/*
 * For debugging, only the least significant byte of the above.
 * Typically it is unique enough for small swarms of radios.
 */
uint8_t System::shortID() {
	return (uint8_t) ID();
}


