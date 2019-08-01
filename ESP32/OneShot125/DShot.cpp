/*
 * DShot.cpp
 *
 *  Created on: 3 Jun 2019
 *      Author: xasin
 */

#include "xasin/DShot.h"

namespace Xasin {
namespace OneShot125 {

#define DSHOT_PRESCALER 5

#define DSHOT_CLOCK_PER_NS ((80000000.0/1000000000.0)/DSHOT_PRESCALER)

#define DSHOT_ZERO_HIGH_NS	(2500)
#define DSHOT_ONE_HIGH_NS 	(5000)
#define DSHOT_TOTAL_NS		(6680)

DShot::DShot(gpio_num_t pin, rmt_channel_t channel) :
		currentTXPin(pin),
		rmtChannel(channel), rmt_buffer(),
		current_throttle_value(0) {

	esp_pm_lock_create(ESP_PM_APB_FREQ_MAX, 0, NULL, &powerLock);
}

DShot::~DShot() {
	esp_pm_lock_delete(powerLock);

	rmt_driver_uninstall(rmtChannel);
	gpio_reset_pin(currentTXPin);
}

void DShot::init() {
	gpio_reset_pin(currentTXPin);

	rmt_config_t cfg = {};
	rmt_tx_config_t tx_cfg = {};

	tx_cfg.idle_level = RMT_IDLE_LEVEL_LOW;
	tx_cfg.idle_output_en = true;
	tx_cfg.loop_en = false;

	tx_cfg.carrier_en = false;
	tx_cfg.carrier_level = RMT_CARRIER_LEVEL_HIGH;

	cfg.tx_config = tx_cfg;

	cfg.rmt_mode = RMT_MODE_TX;
	cfg.clk_div  = DSHOT_PRESCALER;

	cfg.gpio_num = currentTXPin;
	cfg.mem_block_num = 1;
	cfg.channel = rmtChannel;

	rmt_config(&cfg);
	rmt_driver_install(rmtChannel, 0, 0);
}

void DShot::send_cmd(dshot_cmd_t cmd) {
	uint8_t cmd_val = static_cast<uint8_t>(cmd);

	if(cmd > 47)
		return;

	for(uint8_t i=0; i<10; i++) {
		raw_send(cmd_val, true, true);
	}

	raw_send(current_throttle_value);
}

void DShot::raw_send(uint16_t throttle, bool telemetry, bool wait) {
	rmt_item32_t low = {};
	low.duration0 = (DSHOT_ZERO_HIGH_NS)*DSHOT_CLOCK_PER_NS;
	low.level0    = 1;
	low.duration1 = (DSHOT_TOTAL_NS-DSHOT_ZERO_HIGH_NS)*DSHOT_CLOCK_PER_NS;
	low.level1	  = 0;

	rmt_item32_t high = {};
	high.duration0 = (DSHOT_ONE_HIGH_NS)*DSHOT_CLOCK_PER_NS;
	high.level0    = 1;
	high.duration1 = (DSHOT_TOTAL_NS-DSHOT_ONE_HIGH_NS)*DSHOT_CLOCK_PER_NS;
	high.level1	  = 0;

	rmt_item32_t pause = {};
	pause.duration0 = DSHOT_CLOCK_PER_NS * 1000000;
	pause.duration1 = 1;
	pause.level0 = 0;
	pause.level1 = 0;

#pragma pack(1)
	union {
		uint16_t reg;
		struct {
			uint32_t checksum:4;
			uint32_t telemetry:1;
			uint32_t throttle:11;
		};
	} data = {};
#pragma pack(0)

	data.checksum  = 0;
	data.telemetry = telemetry;
	data.throttle  = throttle;

	uint8_t ch_data = 0xF;
	for(uint8_t i=0; i<4; i++) {
		ch_data ^= 0xF & (data.reg >> (i*4));
	}

	data.checksum = ch_data;

	uint16_t dRaw = data.reg;
	for(uint8_t i=0; i<16; i++) {
		(rmt_buffer[i]).val = ((dRaw & (1<<15)) == 0) ? low.val : high.val;

		dRaw <<= 1;
	}

	rmt_buffer[16] = (pause);

	esp_pm_lock_acquire(powerLock);
	rmt_write_items(rmtChannel, rmt_buffer.data(), 17, true);
	esp_pm_lock_release(powerLock);
}

void DShot::send(uint16_t throttle) {
	raw_send(throttle, false, false);
	current_throttle_value = throttle;
}

} /* namespace XIRR */
} /* namespace Xasin */
