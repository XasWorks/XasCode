/*
 * BatteryService.cpp
 *
 *  Created on: 24 Oct 2018
 *      Author: xasin
 */

#include "BLEHandler.h"
#include "services/BatteryService.h"

namespace Xasin		{
namespace Bluetooth {

BatteryService::BatteryService(BLE_Handler *handler, uint16_t vLow, uint16_t vHigh) :
	Service(handler),
	battery_low(vLow), battery_high(vHigh),
	c_battery_percent(this), c_battery_total(this) {

	setBatLevel(vLow);

	set_uuid16(0x180F);

	c_battery_percent.set_uuid16(0x2A19);
	c_battery_percent.set_value(&battery_percent, 1, 1);
	c_battery_percent.can_read(true);
	c_battery_percent.can_write(false);

	c_battery_total.set_uuid32(0x2A1A);
	c_battery_total.set_value(&battery_level, 2, 2);
	c_battery_total.can_read(true);
	c_battery_total.can_write(false);

	this->add_char(&c_battery_percent); this->add_char(&c_battery_total);

	handler->add_service(this);
}

void BatteryService::setBatLevel(uint16_t voltage) {
	battery_level   = voltage;
	int16_t raw_percent = ((voltage - battery_low) * 100) / (battery_high - battery_low);
	if(raw_percent < 0)
		battery_percent = 0;
	if(raw_percent > 100)
		battery_percent = 100;
	else
		battery_percent = raw_percent;
}

} /* namespace Bluetooth */
} /* namespace Peripheral */
