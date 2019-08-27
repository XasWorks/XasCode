/*
 * DShot.cpp
 *
 *  Created on: 3 Jun 2019
 *      Author: xasin
 */

#include "xasin/DShot.h"
#include <cmath>

namespace Xasin {
namespace Drone {

#define DSHOT_PRESCALER 5

#define DSHOT_CLOCK_PER_NS ((80000000.0/1000000000.0)/DSHOT_PRESCALER)

#define DSHOT_ZERO_HIGH_NS	(2500)
#define DSHOT_ONE_HIGH_NS 	(5000)
#define DSHOT_TOTAL_NS		(6666)

DShot::DShot(rmt_channel_t channel, uint8_t channel_count, uint8_t gpio_start) :
		BLDCHandler(channel_count, gpio_start),
		rmtChannel_start(channel), rmt_buffer(),
		throttle_values(channel_count),
		direction_switch_counter(channel_count) {

	esp_pm_lock_create(ESP_PM_APB_FREQ_MAX, 0, NULL, &powerLock);
}

DShot::~DShot() {
	esp_pm_lock_delete(powerLock);

	for(int i=0; i<num_channels; i++) {
		rmt_driver_uninstall(static_cast<rmt_channel_t>(rmtChannel_start + i));
		gpio_reset_pin(static_cast<gpio_num_t>(gpio_start + i));
	}
}

void DShot::init_channel(int chNum) {
	gpio_reset_pin(static_cast<gpio_num_t>(gpio_start + chNum));

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

	cfg.gpio_num = static_cast<gpio_num_t>(gpio_start + chNum);
	cfg.mem_block_num = 1;
	cfg.channel = static_cast<rmt_channel_t>(rmtChannel_start + chNum);

	rmt_config(&cfg);
	rmt_driver_install(static_cast<rmt_channel_t>(rmtChannel_start + chNum), 0, 0);
}

void DShot::init() {
	for(int i=0; i<num_channels; i++)
		init_channel(i);
}

void DShot::send_cmd(uint8_t id, dshot_cmd_t cmd) {
	uint8_t cmd_val = static_cast<uint8_t>(cmd);

	if(cmd > 47)
		return;

	raw_send(id, cmd_val, cmd_val != 0, true);
}

void DShot::raw_send(int chNum, uint16_t throttle, bool telemetry, bool wait) {
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

	uint8_t ch_data = 0;

	for(uint8_t i=0; i<4; i++) {
		ch_data ^= 0xF & (data.reg >> (i*4));
	}

	data.checksum = ch_data & 0xF;

	uint16_t dRaw = data.reg;
	for(uint8_t i=0; i<16; i++) {
		(rmt_buffer[i]).val = ((dRaw & (1<<15)) == 0) ? low.val : high.val;

		dRaw <<= 1;
	}

	rmt_buffer[16] = {};

	esp_pm_lock_acquire(powerLock);
	rmt_write_items(static_cast<rmt_channel_t>(rmtChannel_start + chNum), rmt_buffer.data(), 16, true);
	esp_pm_lock_release(powerLock);
}

void DShot::set_motor_power(uint8_t id, float value) {
	if(id > num_channels)
		return;

	if(value >= 0.99)
		value = 0.99;
	if(value <= -0.99)
		value = -0.99;

	if(fabs(value) < DSHOT_MIN_THROTTLE) {
		value = DSHOT_MIN_THROTTLE * ((throttle_values[id] < 0) ? -1 : 1);
	}

	throttle_values[id] = value;

	uint16_t throttle = 0;

	if(value > 0)
		throttle = 48 + 1000*value;
	else
		throttle = 1048 - 1000*value;

	if(throttle < (48 + 2000*DSHOT_MIN_THROTTLE))
		throttle = (48 + 2000*DSHOT_MIN_THROTTLE);
	if(throttle > 2048)
		throttle = 2048;

	raw_send(id, throttle, false, false);
}

float DShot::get_motor_power(uint8_t id) {
	if(id > num_channels)
		return 0;
	if(fabs(throttle_values[id]) <= (1.01 * DSHOT_MIN_THROTTLE))
		return 0;

	return throttle_values[id];
}

} /* namespace XIRR */
} /* namespace Xasin */
