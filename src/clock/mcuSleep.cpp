
#include "mcuSleep.h"
#include <cassert>


// platform lib nRF5x
#include <drivers/mcu.h>






void MCUSleep::untilAnyEvent()              { MCU::sleepUntilEvent(); }
void MCUSleep::untilInterrupt()             { MCU::sleepUntilInterrupt(); }



#ifdef NOT_USED

For some reason, this gives link error undefined reference to sd_app_evt_wait

#include <inttypes.h>
extern "C" {
uint32_t sd_app_evt_wait(void);
}
// NRF SDK
#include "nrf_soc.h"	// sd_app_evt_wait
void MCUSleep::untilInterruptSDCompatible() {
	/*
	 * Not call nRF5x::MCU:: since it is SD agnostic
	 *
	 * Instead use function from SDK.  This is a SVC call, inlined from .h
	 */
	unsigned int errorCode = sd_app_evt_wait();
	// always return NRF_SUCCESS, according to docs
	(void) errorCode;

}
#endif


