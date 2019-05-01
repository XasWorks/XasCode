/*
 * LSM6DS3.cpp
 *
 *  Created on: 1 May 2019
 *      Author: xasin
 */

#include "xasin/LSM6DS3.h"

namespace Xasin {
namespace I2C {

LSM6DS3::LSM6DS3(uint8_t address)
	: 	maxGReading(0), maxDPSReading(0),
		axes(),
		isInitialized(false),
		addr(address) {

}

void LSM6DS3::send_cmd(uint8_t cmd, uint8_t value) {
	auto i2c = XaI2C::MasterAction(addr);
	i2c.write(cmd, &value, 1);
	i2c.execute();
}

void LSM6DS3::init() {
	uint8_t FS_XLs[] = {0, 2, 3, 1};

	send_cmd(CTRL1_XL, 0b01000011 | (FS_XLs[maxGReading] << 2));

	send_cmd(CTRL2_G, 0b01000000 | (maxDPSReading << 2));
	send_cmd(CTRL7_G, 0b01000000);	// Enable Highpass at 0.0324Hz

	send_cmd(CTRL3, 0b00110100);	// INT pins Active Low & OD

	send_cmd(CTRL10, 0b00111100);	// Enable extra functions

	isInitialized = true;
}

float LSM6DS3::operator [](uint8_t id) {
	if(id >= 6)
		return 0;

	return axes[id];
}

void LSM6DS3::set_g_max(uint8_t num) {
	if(num == maxGReading)
		return;

	maxGReading = num;

	if(isInitialized) {
		uint8_t FS_XLs[] = {0, 2, 3, 1};
		send_cmd(CTRL1_XL, 0b01000011 | (FS_XLs[maxGReading] << 2));
	}
}

void LSM6DS3::set_dps_max(uint8_t num) {
	if(num == maxDPSReading)
		return;

	maxDPSReading = num;

	if(isInitialized)
		send_cmd(CTRL2_G, 0b01000000 | (maxDPSReading << 2));
}

void LSM6DS3::update() {
	int16_t bufferAxis[6] = {};

	auto i2c = XaI2C::MasterAction(addr);
	i2c.read(OUTX_L_G, bufferAxis, sizeof(bufferAxis));
	i2c.execute();

	float gMax[] = {2, 4, 8, 16};
	float DPSMax[] = {250, 500, 1000, 2000};
	for(uint8_t i=0; i<3; i++) {
		axes[i] = (bufferAxis[3+i]*gMax[maxGReading])/32767;
		axes[3+i] = (bufferAxis[i]*DPSMax[maxDPSReading])/32767;
	}
}

} /* namespace I2C */
} /* namespace Xasin */
