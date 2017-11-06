#include <cassert>

#include "radio.h"
#include "radioData.h"


/*
 * Configuration
 *
 * A. Radio POR power-on-reset resets device configuration to default values.
 *
 * B. RADIO->POWER does NOT affect power.  It only resets configuration when toggled.
 * Radio power is managed by chip, and that does not reset radio configuration.
 *
 * C. Configuration can only be done when radio is DISABLED.
 * (except for certain registers, EVENT regs, PACKETPTR register, see elsewhere.)
 *
 * D. Default (POR) configuration of radio is non-functional (No packetPtr, etc.)
 * You MUST configure it.
 */


using namespace RadioData;


//#define LONG_MESSAGE 1
#define MEDIUM_MESSAGE 1


namespace {
   // POR resets configuration of radio
   bool isConfiguredState = false;
}


void Radio::configurePhysicalProtocol() {

	// Radio must be in disabled state to configure
	assert(!isInUse());


	// Specific to the protocol, here rawish
	RadioData::device.configureFixedFrequency(FrequencyIndex);
	device.configureFixedLogicalAddress();
	device.configureNetworkAddressPool();
#ifdef LONG_MESSAGE
	device.configureMediumCRC();
	device.configureStaticPacketFormat(FixedPayloadCount, LongNetworkAddressLength);
#endif
#ifdef MEDIUM_MESSAGE
	device.configureShortCRC();		// OR LongCRC
	device.configureStaticPacketFormat(FixedPayloadCount, MediumNetworkAddressLength);
#endif
	device.setShortcutsAvoidSomeEvents();
	device.configureMegaBitrate(MegabitRate);
	device.configureFastRampUp();

	// Must follow configureStaticPacketFormat, which destroys PCNF1 register
	device.configureWhiteningOn();
	/*
	 * Convention: whitening seed derived from frequency.
	 * All units must use same whitening seed.
	 */
	device.configureWhiteningSeed(2);

	// !!! DMA set up later, not here.

	// Other parameters are defaulted, possible set later by RadioUseCase

	// Default mode i.e. bits per second
	assert(device.frequency() == FrequencyIndex);
	isConfiguredState = true;
}


bool Radio::isConfigured() { return isConfiguredState; }


void Radio::configureXmitPower(TransmitPowerdBm dBm) {
	// Radio not configurable while in use
	assert(!isInUse());

	/*
	 * Adaption
	 *
	 * Simple cast depends on radioSoc enum values equal device values.
	 */
	device.configureXmitPower((unsigned int) dBm);
}

bool Radio::isValidXmitPower(TransmitPowerdBm dBm) {

	bool result = false;
	switch(dBm) {
	case TransmitPowerdBm::Plus4:
	case TransmitPowerdBm::Plus0:
	case TransmitPowerdBm::Minus4:
	case TransmitPowerdBm::Minus8:
	case TransmitPowerdBm::Minus12:
	case TransmitPowerdBm::Minus16:
	case TransmitPowerdBm::Minus20:
	case TransmitPowerdBm::Minus40:
		result = true;
	}
	return result;
}


TransmitPowerdBm Radio::getXmitPower() {
	// Types are both int8_t but need cast??
	return static_cast<TransmitPowerdBm> (device.getXmitPower());
}
