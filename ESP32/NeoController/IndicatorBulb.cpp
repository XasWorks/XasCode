/*
 * IndicatorBulb.cpp
 *
 *  Created on: 16 Apr 2019
 *      Author: xasin
 */

#include "xnm/neocontroller.h"
#include "freertos/FreeRTOS.h"

#undef CONFIG_XNM_NEO_IND_IDLE_REPEAT
#define CONFIG_XNM_NEO_IND_IDLE_REPEAT 24

namespace XNM {
namespace Neo {

uint32_t get_flashcycle_tick() {
	return xTaskGetTickCount() / (350/portTICK_PERIOD_MS); //(CONFIG_XNM_NEO_IND_SPEED/portTICK_PERIOD_MS);
}

IndicatorBulb::IndicatorBulb() :
		current(), 
		deactivateAfter(0),
		last_brightness(0),
		current_brightness(0),
		target(), mode(OFF), flash_fill(8) {
}

IndicatorBulb& IndicatorBulb::operator =(const bulb_config_t &config) {
	this->target = config.target;
	this->mode = config.mode;
	this->flash_fill = config.flash_fill;

	return *this;
}

void IndicatorBulb::set(Color target, bulb_mode_t mode, TickType_t deactivateTicks, uint8_t fill) {
	this->target = target;
	this->mode = mode;
	this->flash_fill = fill;

	deactivate_after(deactivateTicks);
}

uint8_t IndicatorBulb::pattern_brightness() {
	if ((deactivateAfter != 0) && (deactivateAfter < xTaskGetTickCount())) {
		mode = OFF;
		deactivateAfter = 0;
	}

	switch(mode) {
	default:
		return 0;

	case OFF:
		return 0;

	case IDLE:
		return ((get_flashcycle_tick() % CONFIG_XNM_NEO_IND_IDLE_REPEAT) < 2) ? 180 : 0;
	
	case RUNNING:
		return ((get_flashcycle_tick() % CONFIG_XNM_NEO_IND_IDLE_REPEAT) >= 2) ? 180 : 40;

	case ON:
		return 255;

	case HFLASH:
		return (((2*get_flashcycle_tick() / CONFIG_XNM_NEO_IND_IDLE_REPEAT) % 2 ) == 0) ? 200 : 0;

	case FLASH:
		return (get_flashcycle_tick() / 2) % 2 == 0 ? 255 : 0;
	
	case DFLASH:
		return (get_flashcycle_tick()) % 2 == 0 ? 255 : 0;

	case VAL_RISING:
		return ((get_flashcycle_tick() / 4) % 4 * 255) / 4;
	
	case VAL_FALLING:
		return 255 - ((get_flashcycle_tick() / 4) % 4 * 255) / 4;
	}
}

int IndicatorBulb::switch_tick() {
	auto next_brightness = pattern_brightness();
	if(next_brightness == current_brightness)
		return 0;

	last_brightness = current_brightness;
	current_brightness = next_brightness;

	if(next_brightness > last_brightness)
		return 1;
	if(next_brightness < last_brightness)
		return -1;

	return 0;
}

Color IndicatorBulb::color_tick() {
	auto c_copy = target;
	c_copy.alpha = uint16_t(c_copy.alpha) * current_brightness / 255; 

	if(last_brightness < current_brightness)
		return current.merge_transition(c_copy, 200*255);
	else
		return current.merge_transition(c_copy, 120*255);
}

Color IndicatorBulb::get_color() {
	return current;
}

void IndicatorBulb::deactivate_after(TickType_t ticks) {
	if(ticks <= 0)
		return;

	this->deactivateAfter = ticks + xTaskGetTickCount();
}

}
}
