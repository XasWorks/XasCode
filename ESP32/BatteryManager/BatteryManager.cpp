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
	: current_mv_var(3600),
	  cutoff_voltage(3500),
	  is_charging(false) {

	cutoff_percentage = raw_capacity_for_voltage(cutoff_voltage);
}

uint8_t BatteryManager::raw_capacity_for_voltage(uint16_t millivolts) {
	if(millivolts >= discharge_curve[0].voltage)
			return discharge_curve[0].percentage;

	auto lastCharge = discharge_curve[0];
	for(auto c : discharge_curve) {
		if(millivolts == c.voltage)
			return c.percentage;

		if(millivolts > c.voltage) {
			uint16_t mVPlus = millivolts - c.voltage;

			uint16_t mVSpan = lastCharge.voltage - c.voltage;
			uint8_t  percentSpan = lastCharge.percentage - c.percentage;

			return (percentSpan * mVPlus / mVSpan) + c.percentage;
		}
		lastCharge = c;
	}

	return 0;
}

uint8_t BatteryManager::capacity_for_voltage(uint16_t millivolts) {
	uint8_t raw_percent = raw_capacity_for_voltage(millivolts);
	if(raw_percent < cutoff_percentage)
		return 0;

	return 100 - (100-raw_percent)*100 / (100-cutoff_percentage);
}

uint16_t BatteryManager::voltage_for_raw_capacity(uint8_t percentage) {
	if(percentage >= 100)
		return discharge_curve[0].voltage;

	auto lastCharge = discharge_curve[0];
	for(auto c : discharge_curve) {
		if(percentage == c.percentage)
			return c.voltage;

		if(percentage > c.percentage) {
			uint8_t 	pSpan 	= lastCharge.percentage - c.percentage;
			uint16_t 	mvSpan	= lastCharge.voltage	- c.voltage;

			uint8_t		pPlus	= percentage - c.percentage;

			return (pPlus * mvSpan) / pSpan + c.voltage;
		}

		lastCharge = c;
	}

	return discharge_curve[discharge_curve.size()-1].voltage;
}
uint16_t BatteryManager::voltage_for_capacity(uint8_t percentage) {
	return voltage_for_raw_capacity(100 - (100 - percentage) * (100 - cutoff_percentage) / 100);

}

void BatteryManager::set_voltage(uint16_t millivolts) {
	current_mv_var = millivolts;
}

} /* namespace Housekeeping */
