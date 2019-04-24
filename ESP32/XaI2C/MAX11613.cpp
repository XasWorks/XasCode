/*
 * MAX11613.cpp
 *
 *  Created on: 19 Apr 2019
 *      Author: xasin
 */

#include "xasin/MAX11613.h"

namespace Xasin {
namespace I2C {

MAX11613::MAX11613(i2c_port_t port) : port(port) {
}

void MAX11613::init() {
	auto i2c = XaI2C::MasterAction(0b110100);

	i2c.write(0b11010000);
	i2c.write(0b00000111);
	i2c.execute(port);
}

void MAX11613::update() {
	auto i2c = XaI2C::MasterAction(0b110100);

	i2c.read(readings.data(), readings.size()*2);
	i2c.execute(port);

	for(uint8_t i=0; i<readings.size(); i++) {
		uint16_t rawRead = readings[i];
		readings[i]  = (rawRead & (255<<8)) >> 8;
		readings[i] |= (rawRead & 255) << 8;

		readings[i] &= 4095;
	}
}

float MAX11613::get_reading(uint8_t i) {
	if(i > readings.size())
		return 0;
	return readings[i] / (4065.0) * 2.048;
}

} /* namespace I2C */
} /* namespace Xasin */
