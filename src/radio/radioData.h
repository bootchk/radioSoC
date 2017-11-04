
#pragma once



// platform lib e.g. nRF5x
#include <drivers/radio/radio.h>

/*
 * Data (not a class) associated with Radio but not kept by RadioDevice.
 */
namespace RadioData {

// Singleton.  Device is not shared.
extern RadioDevice device;

// App's callback
extern void (*aRcvMsgCallback)();	// = nullptr;

// timestamp of packet
extern LongTime _timeOfArrival;

// used for assertions
extern RadioState state;

/*
 * Buffer R/W by concurrent radio HW (volatile) using DMA.
 * No guards around buffer.
 * We pass address and length to radio HW and it does NOT write outside the buffer.
 * We also pass address and length to Serializer.
 */
extern volatile uint8_t radioBuffer[Radio::FixedPayloadCount];

}
