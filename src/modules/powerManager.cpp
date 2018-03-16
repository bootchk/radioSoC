
#include "powerManager.h"
#include "powerMonitor.h"

// platform lib e.g. nRF5x
#include <drivers/adc.h>



#if defined(NRF52832_XXAA) || defined(NRF52810_XXAA) || defined(NRF51)
#else
#error "NRF51 or NRF52810_XXAA or NRF52832_XXAA not defined"
#endif


namespace {

/*
 * Uses PowerMonitor and ADC, somewhat arbitrarily.
 *
 * You could use only the ADC (on nrf51) or SAADC (on nrf52), could define whatever levels you wish.
 *
 * On the nrf52 you could use only the PowerMonitor which defines more levels, possible as high as 3.6V needed for isPowerExcess.
 */
PowerMonitor powerMonitor;

#ifdef NRF51
ADC adc;
#endif
}  // namespace



/*
 * General discussion of a context.
 * Assume power supply is a solar cell to a capacitor and not a battery.
 * Capacitor has certain joules per delta V.
 * Solar cell delivers about 10uA @2.4V (in 200 lux.)
 * Solar cell can deliver 4.8V (in full sun) which is excess over Vmax of 3.6V
 * Mcu Vbrownout (aka Vmin) is 1.9V but typically can work down to 1.7V.
 *
 * Voltage levels indicate power reserves on the capacitor.
 *
 * The caller is responsible for assigning meaning to the voltage levels,
 * and managing power use.
 * Need say 2.1V on capacitor to burst the radio without real brownout.
 * Need say 2.3V on capacitor to burst the work without without real brownout.
 *
 * An external voltage monitor chip may POR the mcu at 2.0V,
 * but have a hysteresis so that it does not power off the mcu until say 1.8V.
 */

/*
 * Levels
 *
 * Constrained by what underlying device implements.
 */

void PowerManager::init() {

#ifdef NRF51
	adc.init();
#endif

	PowerMonitor::initBrownoutCallback();
}


void PowerManager::enterBrownoutDetectMode() {
	// PowerMonitor does it
	powerMonitor.enterBrownoutDetectMode();
}


// Implemented using ADC or SAADC
bool PowerManager::isPowerExcess() {
	// adc differs by family: NRF51 ADC, NRF52 SAADC
	// There is no adc device common to both families
	bool result;
#if defined(NRF52832_XXAA) || defined(NRF52810_XXAA)
	// By returning false, disable all app logic to shed power and prevent Vcc>Vmax
	// result = false;

	result = powerMonitor.isVddGreaterThanThreshold(PowerThreshold::V2_8);

#elif NRF51
	ADCResult value = adc.getVccProportionTo255();
	// Need to use value smaller than 0xFF? say 3.4V
	// This is fragile: must use >= since value never greater than ADC::Result3_6V
	result = (value >= ADC::Result3_6V);
#endif
	return result;
}

bool PowerManager::isPowerNearExcess() {
	bool result;
#if defined(NRF52832_XXAA) || defined(NRF52810_XXAA)
	// By returning false, disable all app logic to shed power and prevent Vcc>Vmax
	// result = false;

	result = powerMonitor.isVddGreaterThanThreshold(PowerThreshold::V2_8);

#elif NRF51
	ADCResult value = adc.getVccProportionTo255();
	// Need to use value smaller than 0xFF? say 3.4V
	// This is fragile: must use >= since value never greater than ADC::Result3_6V
	// TODO nearness a parameter defined elsewhere instead of here just 0.2V
	result = (value >= ADC::Result3_4V);
#endif
	return result;
}



bool PowerManager::isPowerAboveUltraHigh(){
	bool result;
#if defined(NRF52832_XXAA) || defined(NRF52810_XXAA)
	result = powerMonitor.isVddGreaterThanThreshold(PowerThreshold::V2_8);
#elif NRF51
	ADCResult value = adc.getVccProportionTo255();
	result = (value >= ADC::Result3_2V);
#endif
	return result;
}

// Implemented using POFCON
bool PowerManager::isPowerAboveHigh()     { return powerMonitor.isVddGreaterThanThreshold(PowerThreshold::V2_7);}
bool PowerManager::isPowerAboveMedium()   { return powerMonitor.isVddGreaterThanThreshold(PowerThreshold::V2_5);}
bool PowerManager::isPowerAboveLow()      { return powerMonitor.isVddGreaterThanThreshold(PowerThreshold::V2_3);}
bool PowerManager::isPowerAboveUltraLow() { return powerMonitor.isVddGreaterThanThreshold(PowerThreshold::V2_1);}


VoltageRange PowerManager::getVoltageRange() {
	/*
	 * Implementation: step through levels from high to low
	 */
	VoltageRange result;
	// isPowerExcess and isPowerNearExcess might use ADC and take >20uSec
	/*
	Temp  omit excess, we want to catch it before it gets excess
	if (isPowerExcess()) {
		result = VoltageRange::AboveExcess;
	}
	*/
	if (isPowerNearExcess()) {
		result = VoltageRange::NearExcess;
		}
	else if (isPowerAboveHigh()) {
			result = VoltageRange::HighToNearExcess;
		}
	else if (isPowerAboveMedium()) {
		result = VoltageRange::MediumToHigh;
	}
	else if (isPowerAboveLow()) {
		result = VoltageRange::LowToMedium;
	}
	else if (isPowerAboveUltraLow()) {
		result = VoltageRange::UltraLowToLow;
	}
	else {
		// < 2.1V, near brownout of say 1.8V
		result = VoltageRange::BelowUltraLow;
	}

	return result;
}

#ifdef OLD
bool PowerManager::isVoltageExcess() { return ;}

bool PowerManager::isVoltageHigh() {
	return ! powerManager.isVddGreaterThan2_7V()
			& powerManager.isVddGreaterThan2_5V();
}
bool PowerManager::isVoltageMedium() {
	return ! powerManager.isVddGreaterThan2_5V()
			& powerManager.isVddGreaterThan2_3V();
}
bool PowerManager::isVoltageLow() {
	return ! powerManager.isVddGreaterThan2_3V()
			& powerManager.isVddGreaterThan2_1V();
}
bool PowerManager::isVoltageUltraLow() {

	return ! powerManager.isVddGreaterThan2_1V();
}
#endif



