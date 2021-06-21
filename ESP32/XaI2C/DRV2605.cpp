

#include <xasin/xai2c/DRV2605.h>

namespace Xasin {
namespace I2C {

DRV2605::DRV2605(uint8_t address) : 
	rtp_enabled(false), last_rtp(0), addr(address) {

}

esp_err_t DRV2605::autocalibrate_erm() {
	auto i2c = XaI2C::MasterAction(addr);

	uint8_t mode = 0x07;
	i2c.write(MODE, &mode, 1);

	reg_feedback_t feedback = {};
	feedback.bemf_gain = 0;
	feedback.loop_gain = 2;
	feedback.fb_brake_factor = 2;
	feedback.erm_lra_mode = 0;

	i2c.write(FEEDBACK_CTRL, &feedback, 1);

	uint8_t rated_voltage = (4.0 / 0.02133F);
	i2c.write(RATED_VOLTAGE, &rated_voltage, 1);

	uint8_t od_clamp = (255);
	i2c.write(OVERDRIVE_CLAMP, &od_clamp, 1);

	uint8_t go = 0x01;
	i2c.write(GO, &go, 1);

	auto ret = i2c.execute();

	if(ret != ESP_OK)
		return ret;

	vTaskDelay(700);

	return ESP_OK;
}

void DRV2605::rtp_mode() {
	uint8_t mode = 0x5;

	auto i2c = XaI2C::MasterAction(addr);
	i2c.write(MODE, &mode, 1);

	reg_ctrl3_t ctrl3 = {};
	ctrl3.rtp_signed = 1;
	ctrl3.erm_mode = 0;
	i2c.write(CTRL3, &ctrl3, 1);

	reg_ctrl2_t ctrl2 = {};
	ctrl2.bidir_input = 0;
	ctrl2.brake_stabilizer = 1;
	ctrl2.blank_time = 1;
	ctrl2.idiss_time = 1;
	i2c.write(CTRL2, &ctrl2, 1);

	i2c.execute();

	rtp_enabled = true;
}

void DRV2605::sequence_mode() {
	uint8_t mode = 0;

	auto i2c = XaI2C::MasterAction(addr);
	i2c.write(MODE, &mode, 1);

	reg_ctrl3_t ctrl3 = {};
	ctrl3.rtp_signed = 1;
	ctrl3.erm_mode = 1;
	i2c.write(CTRL3, &ctrl3, 1);

	uint8_t lib_sel = 4;
	i2c.write(0x03, &lib_sel, 1);

	i2c.execute();
}

void DRV2605::trig_sequence(uint8_t seq_num) {
	uint8_t go = 1;

	auto i2c = XaI2C::MasterAction(addr);
	i2c.write(0x04, &seq_num, 1);
	i2c.write(GO, &go, 1);

	i2c.execute();
}

void DRV2605::send_rtp(uint8_t rtp_val) {
	if(!rtp_enabled)
		return;

	if(last_rtp == rtp_val)
		return;

	last_rtp = rtp_val;

	auto i2c = XaI2C::MasterAction(addr);
	i2c.write(RTP, &rtp_val, 1);

	i2c.execute();
}
}
}