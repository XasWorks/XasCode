/*
 * MAX11613.h
 *
 *  Created on: 19 Apr 2019
 *      Author: XNM
 */

#ifndef ESP32_XI2C_MAX11613_H_
#define ESP32_XI2C_MAX11613_H_

#include "MasterAction.h"
#include <array>

namespace XNM {
namespace I2C {

class MAX11613 {
private:
	std::array<uint16_t, 4> readings;

public:
	const i2c_port_t port;

	MAX11613(i2c_port_t port = I2C_NUM_0);

	void init();
	void update();

	float get_reading(uint8_t i);
};

} /* namespace I2C */
} /* namespace XNM */

#endif /* ESP32_XI2C_MAX11613_H_ */
