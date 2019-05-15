/*
 * BME680.cpp
 *
 *  Created on: 11 May 2019
 *      Author: xasin
 */

#include "xasin/BME680.h"

#include <cmath>

#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE
#include "esp_log.h"

namespace Xasin {
namespace I2C {

#define SWAP_BYTES(inData) (((inData & 0xFF00) >> 8) | ((inData & 0xFF) << 8))

BME680::BME680(uint8_t addr) :
		calibData(),
		addr(addr) {
}

esp_err_t BME680::send_cmd(uint8_t reg, uint8_t value) {
	auto i2c = XaI2C::MasterAction(addr);

	i2c.write(reg, &value, 1);
	return i2c.execute();
}


#define PTR_SIZE(ptr1, ptr2) (reinterpret_cast<size_t>(ptr1) - reinterpret_cast<size_t>(ptr2))
void BME680::load_calibration() {
	auto i2c_1 = XaI2C::MasterAction(addr);
	i2c_1.read(COEFF1, calibData.coeff1, 25);
	i2c_1.execute();

	auto i2c_2 = XaI2C::MasterAction(addr);
	i2c_2.read(COEFF2, calibData.coeff2, 16);
	i2c_2.execute();

	ESP_LOGI("BME680", "Calibration loaded!");
	ESP_LOG_BUFFER_HEX_LEVEL("BME680", &calibData, sizeof(calibData), ESP_LOG_VERBOSE);

	void *beginPtr = &calibData;
	ESP_LOGD("BME680", "Calibration size is: %d", sizeof(calibData.bits));
	ESP_LOGD("BME680", "Positions: T1 %d T2 %d T3 %d",
			PTR_SIZE(beginPtr, &calibData.bits.T1),
			PTR_SIZE(beginPtr, &calibData.bits.T2),
			PTR_SIZE(beginPtr, &calibData.bits.T3));
}

void BME680::force_measurement() {
	send_cmd(CTRL_MEAS, 0b10010001);
}

#define ERR_CK(expr) do { auto ret = expr; if(ret != ESP_OK) { return ret; } } while(false);
esp_err_t BME680::init_quickstart() {
	ERR_CK(send_cmd(CTRL_MEAS, 0b10010000));
	ERR_CK(send_cmd(CTRL_HUM, 0b00000100));

	ERR_CK(send_cmd(GAS_WAIT, 0x59));
	ERR_CK(send_cmd(RES_HEAT, 0x50));
	ERR_CK(send_cmd(CTRL_GAS1, 0b00010000));

	load_calibration();

	return ESP_OK;
}

bme680_data_t BME680::fetch_data() {
	uint32_t dataBuffer[4] = {0, 0, 0, 0};

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

			dataBuffer[i] = SWAP_BYTES(dataBuffer[i]);
		}
	}

	dataBuffer[0] <<= 4;
	dataBuffer[2] <<= 4;

	ESP_LOGV("BME680", "Fetched:");
	ESP_LOG_BUFFER_HEXDUMP("BME680", dataBuffer, sizeof(dataBuffer), ESP_LOG_VERBOSE);

	lastReading = *reinterpret_cast<bme680_data_t *>(dataBuffer);

	return lastReading;
}

float BME680::get_temp() {

	float v1 = 0;
	float v2 = 0;

	v1  = lastReading.raw_temp / 16384.0;
	v1 -= calibData.bits.T1 / 1024.0;
	v1 *= calibData.bits.T2;

	v2  = lastReading.raw_temp / 131072.0;
	v2 -= calibData.bits.T1 / 8192.0;
	v2  = pow(v2, 2);
	v2 *= calibData.bits.T3  * 16;

	ESP_LOGV("BME680", "Temp data - Linear: %f Quadratic: %f", v1, v2);
	ESP_LOGV("BME680", "Temp factors are T: %d T1: %d T2: %d T3: %d",
			lastReading.raw_temp, calibData.bits.T1, calibData.bits.T2, calibData.bits.T3);

	return (v1 + v2) / 5120.0;
}


} /* namespace I2C */
} /* namespace Xasin */
