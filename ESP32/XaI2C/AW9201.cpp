

#include "xnm/i2c/AW9201.h"
#include "xnm/i2c/MasterAction.h"

#include <esp_log.h>

namespace XNM {
namespace I2C {

AW9201::AW9201() : 
	last_capacitance_value(0), touched(false),
	addr(0x45) {
}

void AW9201::init() {
	auto i2c = XNM::I2C::MasterAction(addr);

	gcr_t gcr = {};
	gcr.enable = 1;

	scan_cfg_t sc = {};
	sc.sc_num = 0x02;
	sc.resolution = 0x07;
	sc.rf_filter = 1;
	sc.idle_wait = 0;
	sc.idle_time = 0;
	sc.max_on_time = 1;

	sc.touch_debounce = 3;

	sc.RESERVED_5 = 5;

	i2c.write(GLOBAL_CFG, &gcr, 1);
	i2c.write(SCAN_CFG_1, &sc, sizeof(sc));

	auto r = i2c.execute();

	if(r != ESP_OK)
		ESP_LOGE("AW9201", "Error, could not initialize AW9201!");
}

void AW9201::update() {
	auto i2c = XNM::I2C::MasterAction(addr);

	isr_t isr;
	uint16_t tmp_sample = 0;

	i2c.read(ISR, &isr, 1);
	i2c.read(SAMP_H, &tmp_sample, 2);

	i2c.execute();

	touched = isr.touch;
	last_capacitance_value = (tmp_sample & 0xFF) << 8 | ((tmp_sample >> 8) & 0xFF);
}

bool AW9201::is_touched() {
	return touched;
}
uint16_t AW9201::cap_value() {
	return last_capacitance_value;
}

}
}