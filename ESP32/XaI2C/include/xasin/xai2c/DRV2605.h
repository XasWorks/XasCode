
#pragma once

#include <stdint.h>
#include <esp_err.h>

#include <MasterAction.h>

#define DRV2605_DEFAULT_ADDR 0x5A
#define DRV2605_RATED_V_30   140

namespace Xasin {
namespace I2C {

class DRV2605 {
public:
	enum reg_addr_t : uint8_t {
		STATUS = 0,
		MODE = 1,
		RTP  = 2,
		LIBRARY = 3,
		GO = 0x0C,
		OVERDRIVE_TIME_OFFSET = 0x0D,
		SUSTAIN_TIME_OFFSET = 0x0E,
		RATED_VOLTAGE = 0x16,
		OVERDRIVE_CLAMP = 0x17,
		FEEDBACK_CTRL = 0x1A,
		CTRL1 = 0x1B,
		CTRL2 = 0x1C,
		CTRL3 = 0x1D,
		CTRL4 = 0x1E,
		VBAT_MON = 0x21,
	};

#pragma pack(1)
	struct reg_mode_t {
		uint8_t mode:3;
		uint8_t :3;
		uint8_t standby:1;
		uint8_t reset:1;
	};

	struct reg_waveform_t {
		uint8_t wave_id:7;
		uint8_t wait:1;
	};

	struct reg_feedback_t {
		uint8_t bemf_gain:2;	// Autocalibrated
		uint8_t loop_gain:2;	// Default: 2
		uint8_t fb_brake_factor:2;
		uint8_t erm_lra_mode:1; // Default OK
	};

	struct reg_ctrl1_t {
		uint8_t drive_time:5;
		uint8_t ac_couple:1;
		uint8_t :1;
		uint8_t startup_boost_en_1;
	};

	// Advanced configuration, leave untouched for the most part!!
	struct reg_ctrl2_t {
		uint8_t idiss_time:2;
		uint8_t blank_time:2;
		uint8_t samp_time:2;
		uint8_t brake_stabilizer:1;
		uint8_t bidir_input:1;
	};

	struct reg_ctrl3_t {
		uint8_t lra_open_loop:1;
		uint8_t pwm_analog:1;
		uint8_t lra_mode:1;
		uint8_t rtp_signed:1; // Defaults to signed, set to 1!
		uint8_t supply_compensation_disable:1;
		uint8_t erm_mode:1; // Defaults to open-loop, set to 0 for closed loop!
		uint8_t noisegate:2;
	};

	struct reg_ctrl4_t {
		uint8_t otp_program_trigger:1;
		uint8_t :1;
		uint8_t otp_status:1;
		uint8_t :1;
		uint8_t auto_cal_time:2; // Defaults to 500ms to 700ms, OK
	};
#pragma pack(0)

	bool rtp_enabled;
	uint8_t last_rtp;

public:
	const uint8_t addr;

	DRV2605(uint8_t address = DRV2605_DEFAULT_ADDR);

	// Initialize and auto-calibrate the system for a standard
	// ERM haptic device
	esp_err_t autocalibrate_erm();

	void rtp_mode();
	void sequence_mode();
	void trig_sequence(uint8_t seq_num);

	void send_rtp(uint8_t value);
};

}
}