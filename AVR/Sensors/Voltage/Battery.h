/*
 * Battery.h
 *
 *  Created on: 04.11.2016
 *      Author: xasin
 */

#ifndef AVR_SENSORS_VOLTAGE_BATTERY_H_
#define AVR_SENSORS_VOLTAGE_BATTERY_H_

#include <avr/io.h>

namespace Voltage {

class Battery {
private:
	const uint8_t pin;

	const uint16_t minVoltage;
	const uint16_t maxVoltage;

	const float convFactor;

	volatile uint16_t measuredVoltage = 0;
	void (*onBatteryCritical)() = 0;

public:
	Battery(uint8_t pin, float maxMeasured, float min, float max);

	float getVoltage();

	void setCriticalFunction(void (*onCritical)());
	bool criticalEnabled = false;

	void update();
	void ADC_update();
};

} /* namespace Voltage */

#endif /* AVR_SENSORS_VOLTAGE_BATTERY_H_ */
