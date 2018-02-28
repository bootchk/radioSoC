
#include "radioXmitPower.h"


/*
 * Must match Nordic docs.
 *
 * Plus we reject some values that the NRF51 does not support (Plus3)
 * and that our prototocol does not use.
 */
TransmitPowerdBm XmitPower::xmitPowerFromRaw(int8_t rawXmitPower) {
	TransmitPowerdBm result;

	switch(rawXmitPower) {
	case 4:  result = TransmitPowerdBm::Plus4; break;
	case 0:  result = TransmitPowerdBm::Plus0; break;
	case -4: result = TransmitPowerdBm::Minus4; break;
	case -8: result = TransmitPowerdBm::Minus8; break;
	case -12: result = TransmitPowerdBm::Minus12; break;
	case -16: result = TransmitPowerdBm::Minus16; break;
	case -20: result = TransmitPowerdBm::Minus20; break;
	case -40: result = TransmitPowerdBm::Minus40; break;
	default: result = TransmitPowerdBm::Invalid; break;
	}
	return result;
}


int8_t XmitPower::rawXmitPower(TransmitPowerdBm xmitPower){
	// Assert that enum values match OTA values
	return static_cast<int8_t> (xmitPower);
}

const char * XmitPower::repr(TransmitPowerdBm xmitPower) {

	const char * result;
	switch(xmitPower) {
		case TransmitPowerdBm::Plus4: result = "4dBm"; break;
		case TransmitPowerdBm::Plus0: result = "0dBm"; break;
		case TransmitPowerdBm::Minus4: result = "-4dBm"; break;
		case TransmitPowerdBm::Minus8: result = "-8dBm"; break;
		case TransmitPowerdBm::Minus12: result = "-12dBm"; break;
		case TransmitPowerdBm::Minus16: result = "-16dBm"; break;;
		case TransmitPowerdBm::Minus20: result = "-20dBm"; break;
		case TransmitPowerdBm::Minus40: result = "-40dBm"; break;
		case TransmitPowerdBm::Invalid: result = "InvaliddBm"; break;
	}
	return result;
}


bool XmitPower::isValidXmitPower(int8_t valueOTA) {

	TransmitPowerdBm xmitdBm = xmitPowerFromRaw(valueOTA);
	return xmitdBm != TransmitPowerdBm::Invalid;
}
