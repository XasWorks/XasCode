/*
 * AS11115.h
 *
 *  Created on: 3 Apr 2019
 *      Author: xasin
 */

#ifndef ESP32_XAI2C_AS1115_H_
#define ESP32_XAI2C_AS1115_H_

#include <array>
#include "MasterAction.h"

namespace Xasin {
namespace I2C {

class AS1115 {
private:
	std::array<uint8_t, 8> segments;

	void send_cmd(uint8_t cmd, uint8_t val);

public:
	const uint8_t address;
	const i2c_port_t i2c_port;

	static void send_self_addressing(i2c_port_t i2c_port = I2C_NUM_0);

	AS1115(uint8_t address = 0b11, i2c_port_t i2cPort = I2C_NUM_0);

	void init();

	void set_segment(uint8_t id, uint8_t code);
	uint16_t get_buttons();

	void update_segments();
};

} /* namespace I2C */
} /* namespace Xasin */

#endif /* ESP32_XAI2C_AS1115_H_ */
