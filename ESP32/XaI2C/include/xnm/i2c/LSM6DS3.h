/*
 * LSM6DS3.h
 *
 *  Created on: 1 May 2019
 *      Author: XNM
 */

#ifndef ESP32_XI2C_LSM6DS3_H_
#define ESP32_XI2C_LSM6DS3_H_

#include "MasterAction.h"
#include <array>

namespace XNM {
namespace I2C {

enum LSM6DS3_Reg : uint8_t {
	INT1_CTRL	= 0x0D,

	CTRL1_XL	= 0x10,

	CTRL2_G		= 0x11,
	CTRL7_G		= 0x16,	// Gyroscope cutoff

	CTRL3		= 0x12,	// INT Pin config

	CTRL10		= 0x19, // Function-enable

	TAP_SRC		= 0x1C,
	D6D_SRC		= 0x1D,
	TAP_CFG		= 0x58,
	TAP_THR		= 0x59,	// Also includes orientation angle

	OUTX_L_G	= 0x22,
	STEP_CNT	= 0x4B,
};

union tap_src_t {
	uint8_t raw;
	struct {
		uint8_t tapAxis:3;
		uint8_t tapDir:1;
		uint8_t doubleTap:1;
		uint8_t singleTap:1;
		uint8_t tapDetect:1;
	};
};

class LSM6DS3 {
private:
	uint8_t  maxGReading;
	uint8_t maxDPSReading;

	std::array<float, 6> axes;

	bool isInitialized;

	esp_err_t send_cmd(uint8_t cmd, uint8_t val);
public:
	const uint8_t addr;

	LSM6DS3(uint8_t address = 0b1101011);

	float operator[](uint8_t id);

	void set_g_max(uint8_t max);
	void set_dps_max(uint8_t max);

	void init();
	void update();

	tap_src_t get_tap();
};

} /* namespace I2C */
} /* namespace XNM */

#endif /* ESP32_XI2C_LSM6DS3_H_ */
