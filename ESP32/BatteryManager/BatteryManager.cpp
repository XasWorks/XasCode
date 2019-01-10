/*
 * BatteryManager.cpp
 *
 *  Created on: 9 Jan 2019
 *      Author: xasin
 */

#include "BatteryManager.h"

#include "LiPoChargeTable.h"

namespace Housekeeping {

BatteryManager::BatteryManager()
	: top_voltage(4200), cutoff_voltage(3600) {

	cutoff_percentage = 0;
	cutoff_percentage = capacity_for_voltage(cutoff_voltage);
}

uint8_t BatteryManager::capacity_for_voltage(uint16_t millivolts) {
	if(millivolts >= top_voltage)
		return 100;

	uint8_t raw_percent = 100;

	CHARGE_POINT lastCharge = {100, 4200};
	for(CHARGE_POINT c : discharge_curve) {
		if(millivolts == c.voltage)
			return 100;

		if(c.voltage >= top_voltage)
			continue;

		if(millivolts > c.voltage) {
			uint16_t mVPlus = millivolts - c.voltage;

			uint16_t mVSpan = lastCharge.voltage - c.voltage;
			uint8_t  percentSpan = lastCharge.percentage - c.percentage;

			raw_percent = (percentSpan * (100*mVPlus) / mVSpan)/100 + c.percentage;
			break;
		}
		lastCharge = c;
	}

	if(raw_percent < cutoff_percentage)
		return 0;

	return 100 - (100-raw_percent)*100 / (100-cutoff_percentage);
}

} /* namespace Housekeeping */
