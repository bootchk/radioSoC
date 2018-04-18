#pragma once

#include <inttypes.h>

/*
 * Fundamental types between platform and SyncAgent.
 */


// TODO separate types for counter, timeout
/*
 * Type used for:
 *
 * 1) Ticks delivered by platform's free-running, circular clock/counter.
 * 2) Timeouts (implemented using counter)
 * The use cases could use a different type
 * (e.g. virtual timer could implement a longer timeout than counter provides.)
 *
 * Called 'OS' but the platform need not have a true OS.
 *
 * On some platforms, only lower 24 bits valid.
 */
// FUTURE implement enforcement in type of 24-bit limitation on some platforms.
// For now, as long as don't schedule more than 512 seconds ahead, should be OK.

typedef uint32_t OSTime;

/*
 * 24-bit max OSTime
 */
static const unsigned int MaxTimeout = 0xFFFFFF;


/*
 * MAC id of radio, unique system/unit identifier.
 * Content of sync msg to identify  master.
 * BT: 48-bits
 */
typedef uint64_t SystemID;	// lower 6 btyes



/*
 * Mail delivered by SyncAgent to and from app.
 *
 * Currently an instance of primitive type.
 */
typedef unsigned char MailContents;



/*
 * Pointer to unsigned bytes writeable concurrently by device and mcu.
 * The pointed-to uint8_t is volatile.
 *
 * Same as RadioBufferPointer
 */
typedef volatile uint8_t * BufferPointer;
