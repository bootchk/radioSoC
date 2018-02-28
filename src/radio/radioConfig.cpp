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
 * Also, Softdevice may reset radio and its configuration
 */


using namespace RadioData;


//#define LONG_MESSAGE 1
#define MEDIUM_MESSAGE 1

namespace {
// Remember distinct signature of radio configuration for SleepSync protocol
uint32_t configuredSignature;
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

	configuredSignature = device.configurationSignature();

	// Default mode i.e. bits per second
	assert(device.frequency() == FrequencyIndex);
}


/*
 * Implementation is NOT a local state variable,
 * but a sampling test of the radio's registers.
 * Reset (toggling the NRF_RADIO->POWER register)
 */
bool Radio::isConfiguredForSleepSync() {
	return configuredSignature == device.configurationSignature();
}


void Radio::configureXmitPower(TransmitPowerdBm dBm) {
	// Radio not configurable while in use
	assert(!isInUse());

	// Adaption from enum to OTA vale
	device.configureXmitPower( XmitPower::rawXmitPower(dBm));
}




TransmitPowerdBm Radio::getXmitPower() {
	return XmitPower::xmitPowerFromRaw(device.getXmitPower());
	// OLD return static_cast<TransmitPowerdBm> (device.getXmitPower());
}
