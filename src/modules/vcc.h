
#pragma once

/*
 * Knows how to measure Vcc i.e. system voltage
 *
 * adc differs by family: NRF51 ADC, NRF52 SAADC
 * There is no adc device common to both families
 */


enum VccResult{
	// Since using 8-bit resolution
	Result3_6V = 255,
	Result3_4V = 240,
	Result3_2V = 225,
	Result2_8V = 198,
	Result2_7V = 191	// NRF52DK typical Vcc is greater than this
};


class Vcc {
public:
	static void init();
	static unsigned int measure();
};
