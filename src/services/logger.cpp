
#include <inttypes.h>
#include <stdarg.h>			// variadic

#include "logger.h"

// Only using one buffer.
#define BUFFER_INDEX 0


// may be defined in Makefilenrf52 -DLOGGING
#ifdef LOGGING
// from NRF_SDK/external/segger_rtt
#include <SEGGER_RTT.h>

void RTTLogger::init() {
	SEGGER_RTT_Init();
}

void RTTLogger::log(const char* aString) {
	SEGGER_RTT_WriteString(BUFFER_INDEX, aString);
}


//I had trouble with this being undefined in linker, name mangled to logPK...
void RTTLogger::log(uint8_t value){
	(void) SEGGER_RTT_printf(BUFFER_INDEX, "x%02x", value);
}


void RTTLogger::log(uint32_t value){
	(void) SEGGER_RTT_printf(BUFFER_INDEX, "%u", value);
}

void RTTLogger::log(uint64_t value ){
	// Print 64-bit int as two uint32-t on same line, hex notation
	// FUTURE this should work, but it doesn't????
	//(void) SEGGER_RTT_printf(BUFFER_INDEX, "%x %x\n", *(((uint32_t*) &value) + 1), value);

	// Print pieces on separate lines
	//(void) SEGGER_RTT_printf(BUFFER_INDEX, "%x \n", value);
	//(void) SEGGER_RTT_printf(BUFFER_INDEX, "%x \n", *(((uint32_t*) &value) + 1)  );

	// Print on one line
	(void) SEGGER_RTT_printf(BUFFER_INDEX, "x%04x", *(((uint32_t*) &value) + 1)  );	// MS word
	(void) SEGGER_RTT_printf(BUFFER_INDEX, "%04x", value);	// LS word and newline

}



#else

void RTTLogger::init() {}
void RTTLogger::log(char const* aString) { (void) aString; }
void RTTLogger::log(uint8_t value){ (void) value; }
void RTTLogger::log(uint32_t value ){ (void) value; }
void RTTLogger::log(uint64_t value ){ (void) value; }


#endif


#ifdef FUTURE
??? this should work
// Not exposed by SEGGER_RTT.h
int SEGGER_RTT_vprintf(unsigned BufferIndex, const char * sFormat, va_list * pParamList);

void logPrintf(const char * sFormat, ...) {

}
// Adapt variadic to SEGGER vprintf
void logf(const char* formatString, ...) {
	va_list argp;
	va_start(argp, formatString);
	(void) SEGGER_RTT_vprintf(BUFFER_INDEX, formatString, &argp);
	va_end(argp);
}

#endif
