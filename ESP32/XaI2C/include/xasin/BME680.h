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
};

#pragma pack(1)
struct bme680_data_t {
	uint16_t raw_temp;
	uint16_t raw_humidity;
	uint16_t raw_pressure;
	uint8_t raw_voc;
};
#pragma pack(0)

class BME680 {
private:
	void send_cmd(uint8_t reg, uint8_t val);
	void force_measurement();
public:
	const uint8_t addr;

	BME680(uint8_t address = 0b1110110);

	void init_quickstart();

	bme680_data_t fetch_data();
};

} /* namespace I2C */
} /* namespace Xasin */

#endif /* ESP32_XAI2C_BME680_H_ */
