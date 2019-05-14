/*
 * BME680.h
 *
 *  Created on: 11 May 2019
 *      Author: xasin
 */

#ifndef ESP32_XAI2C_BME680_H_
#define ESP32_XAI2C_BME680_H_

#include "MasterAction.h"

namespace Xasin {
namespace I2C {

enum BME680_Reg : uint8_t {
	CONFIG 		= 0x75,
	CTRL_MEAS 	= 0x74,
	CTRL_HUM	= 0x72,

	CTRL_GAS0	= 0x70,
	CTRL_GAS1	= 0x71,

	GAS_WAIT	= 0x64,
	RES_HEAT	= 0x5A,

	GAS_DATA	= 0x2A,
	HUM_DATA	= 0x25,
	TEMP_DATA	= 0x22,
	PRESS_DATA	= 0x1F,

	COEFF1		= 0x89,
	COEFF2		= 0xE1,
};

#pragma pack(1)
struct bme680_data_t {
	uint32_t raw_temp;
	uint32_t raw_humidity;
	uint32_t raw_pressure;
	uint32_t raw_voc;
};

union bme680_calibration_t {
	struct {
		uint8_t coeff1[25];
		uint8_t coeff2[16];
	};
	struct {
		uint8_t:8;
		int16_t T2;
		int8_t  T3;
		uint16_t P1;
		int16_t P2;
		int8_t	 P3;
		int16_t P4;
		int16_t P5;
		int8_t	 P7;
		int8_t  P6;
		int16_t P8;
		int16_t P9;
		uint8_t  P10;
		uint16_t H2_SWAP;
		uint16_t H1;
		int8_t  H3;
		int8_t  H4;
		int8_t	 H5;
		uint8_t	 H6;
		int8_t  H7;
		uint16_t T1;
		int16_t G2;
		int8_t	 G1;
		int8_t	 G3;
	} bits;
};

#pragma pack(0)

class BME680 {
private:
	bme680_calibration_t calibData;
	bme680_data_t lastReading;

	esp_err_t send_cmd(uint8_t reg, uint8_t val);

	void load_calibration();
public:
	const uint8_t addr;

	BME680(uint8_t address = 0b1110110);

	esp_err_t init_quickstart();

	void force_measurement();
	bme680_data_t fetch_data();

	float get_temp();
};

} /* namespace I2C */
} /* namespace Xasin */

#endif /* ESP32_XAI2C_BME680_H_ */
