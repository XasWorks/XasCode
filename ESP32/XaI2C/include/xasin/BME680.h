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
	SW_ERR		= 0x04,
};

#pragma pack(1)
struct bme680_data_t {
	uint32_t raw_temp;
	uint32_t raw_humidity;
	uint32_t raw_pressure;
	uint32_t raw_voc;
	uint32_t gas_range;
};

union bme680_calibration_t {
	struct {
		uint8_t coeff1[25];
		uint8_t coeff2[16];
	};
	struct {
		uint8_t	DUMMY1;	// 0
		int16_t T2;	// 1 + 2
		int8_t  T3;	// 3
		uint8_t DUMMY2;	// 4
		uint16_t P1;	// 5 + 6
		int16_t P2;	// 7 + 8
		int8_t	 P3;	// 9
		uint8_t DUMMY3;	// 10
		int16_t P4;	// 11 + 12
		int16_t P5;	// 13 + 14
		int8_t	 P7;	// 15
		int8_t  P6;	// 16
		uint16_t DUMMY4;// 17 + 18
		int16_t P8;	// 19 + 20
		int16_t P9;	// 21 + 22
		uint8_t  P10;	// 23
		uint8_t DUMMY5;	// 24
		uint8_t H2_MSB;
		uint8_t H1_H2_LSB;	// 26
		uint8_t H1_MSB;	// 27	
		int8_t  H3;	// 28
		int8_t  H4;	// 29
		int8_t	 H5;	// 30
		uint8_t	 H6;	// 31
		int8_t  H7;	// 32
		uint16_t T1;	// 33 + 34
		int16_t G2;	// 35 + 36
		int8_t	 G1;	// 37
		int8_t	 G3;	// 38
		int8_t	 SW_ERR;
	} bits;
};

#pragma pack(0)

class BME680 {
private:
	bme680_calibration_t calibData;
	bme680_data_t lastReading;

	float t_fine;

	esp_err_t send_cmd(uint8_t reg, uint8_t val);

	void load_calibration();
public:
	const uint8_t addr;
	
	BME680(uint8_t address = 0b1110110);

	esp_err_t init_quickstart();

	void force_measurement();
	bme680_data_t fetch_data();

	float get_temp();
	float get_humidity();
	float get_pressure();
	float get_gas_res();

	float get_air_quality();
};

} /* namespace I2C */
} /* namespace Xasin */

#endif /* ESP32_XAI2C_BME680_H_ */
