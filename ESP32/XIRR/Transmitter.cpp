/*
 * Transmitter.cpp
 *
 *  Created on: 3 Jun 2019
 *      Author: xasin
 */

#include "xasin/xirr/Transmitter.h"

namespace Xasin {
namespace XIRR {

Transmitter::Transmitter(gpio_num_t pin, rmt_channel_t channel) :
		txPin(pin), rmtChannel(channel), rmt_buffer() {

	esp_pm_lock_create(ESP_PM_APB_FREQ_MAX, 0, NULL, &powerLock);
}

Transmitter::~Transmitter() {
	esp_pm_lock_delete(powerLock);

	rmt_driver_uninstall(rmtChannel);
	gpio_reset_pin(txPin);
}

void Transmitter::init() {
	gpio_reset_pin(txPin);

	rmt_config_t cfg = {};
	rmt_tx_config_t tx_cfg = {};

	tx_cfg.idle_level = RMT_IDLE_LEVEL_LOW;
	tx_cfg.idle_output_en = true;
	tx_cfg.loop_en = false;

	tx_cfg.carrier_duty_percent = 50;
	tx_cfg.carrier_freq_hz = 40000;
	tx_cfg.carrier_en = true;
	tx_cfg.carrier_level = RMT_CARRIER_LEVEL_HIGH;

	cfg.tx_config = tx_cfg;

	cfg.rmt_mode = RMT_MODE_TX;
	cfg.clk_div  = 200;
	cfg.gpio_num = txPin;
	cfg.mem_block_num = 1;
	cfg.channel = rmtChannel;

	rmt_config(&cfg);
	rmt_driver_install(rmtChannel, 0, 0);
}

void Transmitter::add_byte(uint8_t data, uint8_t cont) {
	rmt_item32_t low = {};
	low.duration0 = (80000000/200)/2000 - 1;
	low.level0    = 0;
	low.duration1 = 1;
	low.level1	  = 0;

	rmt_item32_t high = low;
	high.level0 = 1;

	for(uint8_t i=0; i<8; i++) {
		rmt_buffer.emplace_back((data & 1) ? high : low);
		data >>= 1;
	}

	if(cont == 1)
		rmt_buffer.emplace_back(high);
	else if(cont == 2)
		rmt_buffer.emplace_back(low);
}

void Transmitter::send(const void *data, size_t length, uint8_t channel) {

	rmt_item32_t start = {};
	start.duration0 = (80000000/200)/2000 * 7;
	start.level0    = 1;
	start.duration1 = (80000000/200)/2000;
	start.level1    = 0;

	rmt_buffer.emplace_back(start);

	uint8_t add_checksum = 0;
	add_byte(channel, 1);
	add_checksum += channel;

	for(size_t i = 0; i<length; i++) {
		uint8_t formatData = *(reinterpret_cast<const uint8_t*>(data)+i);
		add_checksum += formatData;

		add_byte(formatData, 1);
	}
	add_byte(add_checksum, 2);


	esp_pm_lock_acquire(powerLock);
	rmt_write_items(rmtChannel, rmt_buffer.data(), rmt_buffer.size(), true);
	esp_pm_lock_release(powerLock);

	rmt_buffer.clear();
}

} /* namespace XIRR */
} /* namespace Xasin */
