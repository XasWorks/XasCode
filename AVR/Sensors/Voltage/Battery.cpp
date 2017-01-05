/*
 * Battery.cpp
 *
 *  Created on: 04.11.2016
 *      Author: xasin
 */

#include "Battery.h"
#include "../../ADC/ADC_Lib.h"

namespace Voltage {

Battery::Battery(uint8_t pin, float maxMeasured, float min, float max) :
		pin(pin), minVoltage((uint16_t)(min/maxMeasured * 1023)), maxVoltage((uint16_t)(max/maxMeasured * 1023)),
		convFactor(maxMeasured/1023) {
}

float Battery::getVoltage() {
	return this->measuredVoltage * this->convFactor;
}

void Battery::setCriticalFunction(void (*onCritical)()) {
	this->onBatteryCritical = onCritical;
}

void Battery::update() {
	ADC_Lib::start_measurement(this->pin);
}
void Battery::ADC_update() {
	if(ADC_Lib::measuredPin == this->pin) {
		this->measuredVoltage = ADC_Lib::lastResult;

		if(this->criticalEnabled)
			if((measuredVoltage >= maxVoltage || measuredVoltage <= minVoltage))
				this->onBatteryCritical();
	}
}

} /* namespace Voltage */
