
#include "vcc.h"


// platform lib e.g. nRF5x
#if defined(NRF52832_XXAA) || defined(NRF52810_XXAA) || defined(NRF52_SERIES)
   #include <drivers/adc/saadc.h>
#elif defined(NRF51)
   #include <drivers/adc/adc.h>
#else
   #error "NRF51 or NRF52810_XXAA or NRF52832_XXAA not defined"
#endif




void Vcc::init() {
#ifdef NRF51
	ADC::init();
#else
	VccMonitor::init();
#endif
}


VccResult Vcc::measure() {
	VccResult value;

#if defined(NRF52832_XXAA) || defined(NRF52810_XXAA)

	value = static_cast<VccResult> (VccMonitor::getVccProportionTo255());

#elif NRF51
	value = ADC::getVccProportionTo255();
	// Need to use value smaller than 0xFF? say 3.4V
	// This is fragile: must use >= since value never greater than ADC::Result3_6V

#endif
	return value;
}
