#pragma once

#include <inttypes.h>

#ifdef FUTURE

// lkk Can't get this to work
#define NRF_LOG_ENABLED 1
#define NRF_LOG_DEFAULT_LEVEL 0
#define NRF_LOG_USES_UART = 1

 /extern "C" {
ret_code_t nrf_log_init(nrf_log_timestamp_func_t timestamp_func);
}

void initLogging(void)
{
    // Initialize logging library.
	__attribute__((unused)) uint32_t err_code = NRF_LOG_INIT((nrf_log_timestamp_func_t) NULL);
    // APP_ERROR_CHECK(err_code);
}

// Macro definitions.
// Leave NRF_LOG macros in place, these define them null ifndef NRF_LOG_ENABLED
#include "nrf_log.h"	// writing to log
#include "nrf_log_ctrl.h"	// initing log

#endif

class RTTLogger {
public:
	static void init();
	static void log(const char* aString);
	static void log(uint8_t);
	static void log(uint32_t);
	static void log(uint64_t);
};


//void logPrintf(const char * sFormat, ...);

