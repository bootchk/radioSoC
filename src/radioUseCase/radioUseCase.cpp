
#include "radioUseCase.h"

#include "../radio/radio.h"

namespace {

// shadow of value that radio owns
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
	return power;
}
