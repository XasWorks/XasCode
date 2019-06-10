/*
 * AS11115.cpp
 *
 *  Created on: 3 Apr 2019
 *      Author: xasin
 */

#include "xasin/AS1115.h"

namespace Xasin {
namespace I2C {

AS1115::AS1115(uint8_t address, i2c_port_t i2c_port)
	:	segments(),
		address(address), i2c_port(i2c_port) {

}

void AS1115::send_self_addressing(i2c_port_t i2c_port) {
	for(int8_t addr=0b11 + 16; addr >= 0; addr--) {
		auto i2c_cmd = XaI2C::MasterAction(addr & 0b11);

		uint8_t payload = 0;
		i2c_cmd.write(0x0C, &payload, 1); // Power up and reset devices
		i2c_cmd.write(0x2D, &payload, 1);

		vTaskDelay(20);
		i2c_cmd.execute(i2c_port);
	}
}

void AS1115::send_cmd(uint8_t cmd, uint8_t val) {
	auto i2c_cmd = XaI2C::MasterAction(address);

	i2c_cmd.write(cmd, &val, 1);
	i2c_cmd.execute(i2c_port);
}

void AS1115::init() {
	send_cmd(0b1100, 1 | 1<<7); // Start device
	send_cmd(0x0B, 0x07); 	// Full scan range

	send_cmd(0x0A, 0x0F);	// Full global brightness
}

void AS1115::set_segment(uint8_t id, uint8_t code) {
	if(id > segments.size())
		return;

	segments[id] = code;
}

uint16_t AS1115::get_buttons() {
	uint16_t outVal = 0;

	auto i2c_cmd = XaI2C::MasterAction(address);
	i2c_cmd.read(0x1C, &outVal, 2);
	auto ret = i2c_cmd.execute();

	if(ret != ESP_OK)
		return 0;

	return 0xFFFF ^ outVal;
}

void AS1115::update_segments() {
	auto i2c_cmd = XaI2C::MasterAction(address);

	i2c_cmd.write(1, segments.data(), 8);
	i2c_cmd.execute(i2c_port);
}

} /* namespace I2C */
} /* namespace Xasin */
