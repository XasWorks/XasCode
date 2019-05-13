/*
 * BME680.cpp
 *
 *  Created on: 11 May 2019
 *      Author: xasin
 */

#include "xasin/BME680.h"

namespace Xasin {
namespace I2C {

BME680::BME680(uint8_t addr) : addr(addr) {
}

void BME680::send_cmd(uint8_t reg, uint8_t value) {
	auto i2c = XaI2C::MasterAction(addr);

	i2c.write(reg, &value, 1);
	i2c.execute();
}

void BME680::force_measurement() {
	send_cmd(CTRL_MEAS, 0b10010001);
}

void BME680::init_quickstart() {
	send_cmd(CTRL_MEAS, 0b10010000);
	send_cmd(CTRL_HUM, 0b00000100);

	send_cmd(GAS_WAIT, 0x59);
	send_cmd(RES_HEAT, 0x50);
	send_cmd(CTRL_GAS1, 0b00010000);

}

bme680_data_t BME680::fetch_data() {
	uint16_t dataBuffer[4] = {0};

	uint8_t registers[] = {
		TEMP_DATA,
		HUM_DATA,
		PRESS_DATA,
	};

	for(uint8_t i=0; i<4; i++) {
		auto i2c = XaI2C::MasterAction(addr);

		if(i == 3) {
			i2c.read(GAS_DATA, &dataBuffer[3], 1);
			i2c.execute();
		}
		else {
			i2c.read(registers[i], &dataBuffer[i], 2);
			i2c.execute();

			uint16_t swapBuf = dataBuffer[i];
			dataBuffer[i]  = swapBuf >> 8;
			dataBuffer[i] |= swapBuf << 8;
		}

	}

	return *reinterpret_cast<bme680_data_t *>(dataBuffer);
}

} /* namespace I2C */
} /* namespace Xasin */
