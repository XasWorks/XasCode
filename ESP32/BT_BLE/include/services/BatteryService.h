/*
 * BatteryService.h
 *
 *  Created on: 24 Oct 2018
 *      Author: xasin
 */

#include "Service.h"

#ifndef COMPONENTS_BLUETOOTH_SERVICES_BATTERYSERVICE_H_
#define COMPONENTS_BLUETOOTH_SERVICES_BATTERYSERVICE_H_

namespace Xasin		{
namespace Bluetooth {

class BatteryService: public Service {
private:

	uint8_t  battery_percent;
	uint16_t battery_level;

	const uint16_t battery_low;
	const uint16_t battery_high;

	Characteristic c_battery_percent;
	Characteristic c_battery_total;

public:
	BatteryService(BLE_Handler *handler, uint16_t vLow, uint16_t vHigh);

	void setBatLevel(uint16_t voltage);
};

} /* namespace Bluetooth */
} /* namespace Peripheral */

#endif /* COMPONENTS_BLUETOOTH_SERVICES_BATTERYSERVICE_H_ */
