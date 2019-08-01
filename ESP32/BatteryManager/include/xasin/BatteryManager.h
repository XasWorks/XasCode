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

	uint16_t current_mv_var;

public:
	const uint8_t cell_count;
	const uint32_t cutoff_voltage;

	bool is_charging;

	uint8_t		raw_capacity_for_voltage(uint32_t millivolts);
	uint8_t 	capacity_for_voltage(uint32_t millivolts);
	uint32_t	voltage_for_raw_capacity(uint8_t percentage);
	uint32_t 	voltage_for_capacity(uint8_t percentage);

	BatteryManager(uint8_t cellCount = 1);

	void set_voltage(uint32_t millivolts);

	uint32_t current_mv();
	uint8_t  current_capacity();

	bool battery_ok();
};

} /* namespace Housekeeping */

#endif /* XASLIBS_HOUSEKEEPING_BATTERYMANAGER_BATTERYMANAGER_H_ */
