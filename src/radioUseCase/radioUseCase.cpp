
#include "radioUseCase.h"

#include "../radio/radio.h"

namespace {

/*
 * shadow of value that radio owns
 *
 * A reset radio has power Plus0.
 * Thus initial RadioUseCase shadow equals radio.
 */
TransmitPowerdBm power = TransmitPowerdBm::Plus0;
}


void RadioUseCase::applyToRadio(){
	// assert use case is active
	Radio::configureXmitPower(power);
}


void RadioUseCase::setXmitPower(TransmitPowerdBm value) {
	power = value;
	applyToRadio();
}

TransmitPowerdBm RadioUseCase::getXmitPower() {
	// !!! return value from device
	return Radio::getXmitPower();
}
