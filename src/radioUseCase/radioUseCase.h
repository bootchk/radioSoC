#pragma once

#include "../radio/radioTypes.h"


/*
 * Defines protocol and other parameters of a use of the radio.
 *
 * Modal: only one RadioUseCase can be active at a time (since there is only one radio.)
 *
 * Most parameters are static properties of use case.
 *
 * Certain parameters are changeable on the fly as RadioUseCase is active.
 *
 * This understands:
 *  - RadioDevice defaults all parameters
 *  - RadioDevice retains most parameters even in low power
 *  - changing active RadioUseCase requires reconfiguring RadioDevice
 *
 * Legal to use a RadioUseCase whose parameters have not been set
 * (since RadioDevice defaults them.)
 * However, then getters return wrong values
 */

// TODO fix getters return values from RadioDevice

// TODO contain protocol also

/*
 * Not a pure class singleton: instances exist.
 */
class RadioUseCase {

public:

	static void applyToRadio();

	/*
	 * Takes immediate effect.  This use case must be active.
	 *
	 * Persists through deactivation of RadioUseCase.
	 * Does not persist through a reset of cpu.
	 */
	// TODO make persist through reset
	static void setXmitPower(TransmitPowerdBm power);

	// Returns xmit power from device, not any memoized value
	static TransmitPowerdBm getXmitPower();
};
