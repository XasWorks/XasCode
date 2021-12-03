/**
 * @file AW9201.h
 * @author your name (you@domain.com)
 * @brief AW9201 single channel capacitive touch sensor interface
 * @version 0.1
 * @date 2021-12-02
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include <stdint.h>

namespace XNM {
namespace I2C {

class AW9201 {
public:
#pragma pack(1)
	struct gcr_t {
		uint8_t :1;
		uint8_t enable:1;
		uint8_t int_en:1;
		uint8_t :1;
		uint8_t int_mode:1;
	};

	struct isr_t {
		uint8_t :1;
		uint8_t touch:1;
	};

	struct scan_cfg_t {
		uint8_t sc_num:4; // Default 0x02
		uint8_t resolution:4; // Default 0x07
		uint8_t sc_offset:5; 
		uint8_t :1;
		uint8_t rf_filter:2;
		uint8_t idle_wait:3;  // Default 0x02
		uint8_t :1;
		uint8_t idle_time:2; 
		uint8_t max_on_time:2; 

		uint8_t :2;
		uint8_t touch_debounce:2;
		uint8_t RESERVED_5:4; // MUST be set to 5...?
	};
#pragma pack(0)
	enum reg_addr_t : uint8_t {
		IDRST = 0,
		GLOBAL_CFG = 1,
		ISR = 2,
		TOUCH_STHRESH = 0x04,	// Default 0x20
		TOUCH_CTHRESH = 0x05,	// Default 0x14
		SCAN_CFG_1 = 0x06,

		SAMP_H = 0x22,
		SAMP_L = 0x23,
	};

	uint16_t last_capacitance_value;
	bool touched;

public:
	const uint8_t addr;

	AW9201();

	void init();

	void update();

	bool is_touched();
	uint16_t cap_value();
};

}
}