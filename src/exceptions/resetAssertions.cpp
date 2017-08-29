#include <cassert>

#include "resetAssertions.h"

#include <drivers/mcu.h>


void assertNoResetsOccurred() {
	assert( !MCU::isResetReason() );
}


void clearResetReason() {
	MCU::clearResetReason();
}
