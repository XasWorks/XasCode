/*
 * NeoController.cpp
 *
 *  Created on: 14 Sep 2018
 *      Author: xasin
 */

#include "NeoController.h"

namespace Peripheral {

static void IRAM_ATTR u8_to_WS2812(const void* source, rmt_item32_t* destination,
	size_t source_size, size_t wanted_elements,
	size_t* translated_size, size_t* translated_items) {

	*translated_size  = 0;
	*translated_items = 0;

	rmt_item32_t bits[2] = {};
	bits[0].duration0 = 0.35 * 20; bits[0].level0 = 1;
	bits[0].duration1 = 0.9  * 20; bits[0].level1 = 0;

	bits[1].duration0 = 0.9  * 20; bits[1].level0 = 1;
	bits[1].duration1 = 0.35 * 20; bits[1].level1 = 0;

	while(*translated_size < source_size && *translated_items < wanted_elements) {
		for(uint8_t i=0; i<8; i++) {
			destination->val = bits[(*reinterpret_cast<const int8_t *>(source))>>(7-i) & 1].val;

			(*translated_items)++;
			destination++;
		}

		(*translated_size)++;
		source++;
	}
}

NeoController::NeoController(gpio_num_t pin, rmt_channel_t channel, uint8_t length) :
		length(length),
		colors(length), nextColors(length),
		pinNo(pin), channel(channel) {

	rawColors = new Color::ColorData[length];

	clear();
	apply();

	rmt_config_t cfg = {};
	rmt_tx_config_t tx_cfg = {};
	tx_cfg.idle_level = RMT_IDLE_LEVEL_LOW;
	tx_cfg.idle_output_en = true;
	tx_cfg.loop_en = false;
	cfg.tx_config = tx_cfg;

	cfg.channel  = channel;
	cfg.rmt_mode = RMT_MODE_TX;
	cfg.clk_div  = 4;
	cfg.gpio_num = pinNo;
	cfg.mem_block_num = 2;

	rmt_config(&cfg);
	rmt_driver_install(channel, 0, 0);
	rmt_translator_init(channel, u8_to_WS2812);

	esp_pm_lock_create(ESP_PM_APB_FREQ_MAX, 0, NULL, &powerLock);
}

void NeoController::update() {
	esp_pm_lock_acquire(powerLock);
	for(uint8_t i=0; i<length; i++)
		rawColors[i] = colors[i].getLEDValue();

	rmt_write_sample(channel, reinterpret_cast<const unsigned char *>(rawColors), length*sizeof(Color::ColorData), true);

	esp_pm_lock_release(powerLock);
}

void NeoController::fill(Color color) {
	for(uint8_t i=0; i<length; i++)
		nextColors[i] = color;
}
void NeoController::clear() {
	fill(Color());
}

Color& NeoController::operator[](int id) {
	return *get(id);
}
Color * NeoController::get(int id) {
	return &this->nextColors[id%length];
}

void NeoController::apply() {
	for(uint8_t i=0; i<length; i++)
		this->colors[i] = this->nextColors[i];
}

void NeoController::fadeTransition(uint32_t duration) {
	const uint64_t startTime = esp_timer_get_time();

	Color* startColors = new Color[length];
	for(uint8_t i=0; i<length; i++)
		startColors[i] = this->colors[i];

	uint32_t passedTicks = 0;
	while(passedTicks < duration) {
		for(uint8_t i=0; i<length; i++)
			this->colors[i].overlay(startColors[i], nextColors[i], (255*passedTicks)/duration);
		update();
		vTaskDelay((1000/60) / portTICK_PERIOD_MS);

		passedTicks = esp_timer_get_time() - startTime;
	}

	this->apply();
	this->update();

	delete startColors;
}

void NeoController::swipeTransition(uint32_t duration, bool desc) {
	const uint32_t perPixelDuration = duration/length;
	const uint64_t startTime = esp_timer_get_time();

	uint8_t i = 0;
	uint8_t iLast = 0;

	while(i < length) {

		for(uint8_t j = iLast; iLast <= i; iLast++) {
			uint8_t jR = desc ? (length-1 - j) : j;
			colors[jR] = nextColors[jR];
		}

		iLast = i;
		update();

		vTaskDelay((1000/40) / portTICK_PERIOD_MS);
		i = (esp_timer_get_time() - startTime) / perPixelDuration;
		if((esp_timer_get_time() - startTime) > duration)
			break;
	}

	this->apply();
	this->update();
}

} /* namespace Touch */
