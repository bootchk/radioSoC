
#include "radio.h"
#include "radioData.h"


using namespace RadioData;

void Radio::powerOn() { device.powerOn(); }
void Radio::powerOff() { device.powerOff(); }
bool Radio::isPowerOn() { return device.isPowerOn(); }

