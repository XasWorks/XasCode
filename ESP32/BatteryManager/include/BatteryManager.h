/*
 * BatteryManager.h
 *
 *  Created on: 9 Jan 2019
 *      Author: xasin
 */

#ifndef XASLIBS_HOUSEKEEPING_BATTERYMANAGER_BATTERYMANAGER_H_
#define XASLIBS_HOUSEKEEPING_BATTERYMANAGER_BATTERYMANAGER_H_

#include "stdint.h"

namespace Housekeeping {

class BatteryManager {
private:
	uint8_t cutoff_percentage;

public:
	const uint16_t cutoff_voltage;

	uint8_t		raw_capacity_for_voltage(uint16_t millivolts);
	uint8_t 	capacity_for_voltage(uint16_t millivolts);
	uint16_t	voltage_for_raw_capacity(uint8_t percentage);
	uint16_t 	voltage_for_capacity(uint8_t percentage);

	BatteryManager();
};

} /* namespace Housekeeping */

#endif /* XASLIBS_HOUSEKEEPING_BATTERYMANAGER_BATTERYMANAGER_H_ */
