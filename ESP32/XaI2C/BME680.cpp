/*
 * BME680.cpp
 *
 *  Created on: 11 May 2019
 *      Author: xasin
 */

#include "xasin/BME680.h"

#include <cmath>

#define LOG_LOCAL_LEVEL ESP_LOG_INFO
#include "esp_log.h"

namespace Xasin {
namespace I2C {

#define SWAP_BYTES(inData) (((inData & 0xFF00) >> 8) | ((inData & 0xFF) << 8))

BME680::BME680(uint8_t addr) :
		calibData(), lastReading(),
		t_fine(0),
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

	auto i2c_3 = XaI2C::MasterAction(addr);
	i2c_3.read(SW_ERR, &calibData.bits.SW_ERR, 1);

	calibData.bits.SW_ERR >>= 4;

	ESP_LOGI("BME680", "Calibration loaded!");
	ESP_LOG_BUFFER_HEX_LEVEL("BME680", &calibData, sizeof(calibData), ESP_LOG_VERBOSE);
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
		GAS_DATA,
	};

	for(uint8_t i=0; i<4; i++) {
		auto i2c = XaI2C::MasterAction(addr);

		i2c.read(registers[i], &dataBuffer[i], 2);
		i2c.execute();

		dataBuffer[i] = SWAP_BYTES(dataBuffer[i]);
	}

	dataBuffer[0] <<= 4;
	dataBuffer[2] <<= 4;

	uint8_t gRange = dataBuffer[3] & 0x0F;
	dataBuffer[3] >>= 6;


	lastReading = *reinterpret_cast<bme680_data_t *>(dataBuffer);
	lastReading.gas_range = gRange;
	t_fine = -300;

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

	t_fine = v1 + v2;

	return t_fine / 5120.0;
}

float BME680::get_humidity() {
	if(t_fine < -280)
		get_temp();

	float temp_comp = t_fine / 5120.0;

	uint16_t c_h1 = (calibData.bits.H1_MSB << 4) | (calibData.bits.H1_H2_LSB & 0xF);
	uint16_t c_h2 = (calibData.bits.H2_MSB << 4)  | (calibData.bits.H1_H2_LSB >> 4);

	float v1  = lastReading.raw_humidity - c_h1 * 16.0;
		  v1 -= calibData.bits.H3 / 2.0 * temp_comp;

	float v2 = v1 * (c_h2 / 262144.0 * (1.0 + calibData.bits.H4 / 16384.0
		  		* temp_comp + calibData.bits.H5 / 1048576.0 * pow(temp_comp,2)));

	float hum = v2;
		 hum += pow(v2,2) * (calibData.bits.H6/16384.0 + calibData.bits.H7 * temp_comp / 2097152.0);

	if(hum < 0)
		return 0;
	if(hum > 100)
		return 100;

	return hum;
}

float BME680::get_pressure() {
	if(t_fine < -280)
		get_temp();


	float v_lin  = t_fine / 2.0 - 64000.0;
	float v_quad = pow(v_lin, 2) * calibData.bits.P6 / 131072.0;
	v_quad		+= v_lin * calibData.bits.P5 * 2.0;
	v_quad		/= 4.0;
	v_quad		+= calibData.bits.P4 * 65536.0;

	v_lin  = (pow(v_lin, 2) * calibData.bits.P3 / 16384.0 + calibData.bits.P2 * v_lin) / 524288.0;
	v_lin /= 32768.0;
	v_lin += 1;
	v_lin *= calibData.bits.P1;

	float pressure_val = 1048576.0 - lastReading.raw_pressure;

	if(v_lin == 0)
		return 0;

	pressure_val -= v_quad / 4096.0;
	pressure_val *= 6250.0;
	pressure_val /= v_lin;

	v_lin  = calibData.bits.P9 * pow(pressure_val,2) / 2147483648.0;
	v_quad = pressure_val * calibData.bits.P8 / 32768.0;
	float v3 = pow(pressure_val / 256.0, 3) * calibData.bits.P10 / 131072.0;

	return pressure_val + (v_lin + v_quad + v3 + calibData.bits.P7 * 128.0) / 16.0;
}

float BME680::get_gas_res() {
	const float lookup_k1_range[16] = {
	0.0, 0.0, 0.0, 0.0, 0.0, -1.0, 0.0, -0.8,
	0.0, 0.0, -0.2, -0.5, 0.0, -1.0, 0.0, 0.0};
	const float lookup_k2_range[16] = {
	0.0, 0.0, 0.0, 0.0, 0.1, 0.7, 0.0, -0.8,
	-0.1, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};


	float v1 = 1340.0 + 5.0 * calibData.bits.SW_ERR;
	float v2 = v1 * (1.0 + lookup_k1_range[lastReading.gas_range]/100.0);
	float v3 = 1.0 + lookup_k2_range[lastReading.gas_range]/100.0;

	return 1 / (v3 * 0.000000125 * (1<<lastReading.gas_range) *
			((lastReading.raw_voc - 512.0)/v2 + 1));
}

float BME680::get_air_quality() {
	// Quality value in % - higher means better!
	float quality_val = 1;

	// With 45 being optimal, and 30 and 70 being the max.
	quality_val *= 1 - 0.25*pow((get_humidity() - 40)/25.0, 2);

	// Gas resistance quality calculation. 200k is great, 110k kinda normal, <70k a little eh :P
	quality_val *= 1 + 0.6*(get_gas_res() - 110000)/40000;

	return quality_val;
}

} /* namespace I2C */
} /* namespace Xasin */
