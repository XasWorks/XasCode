/*
 * IndicatorBulb.cpp
 *
 *  Created on: 16 Apr 2019
 *      Author: xasin
 */

#include "xasin/neocontroller/IndicatorBulb.h"
#include "freertos/FreeRTOS.h"

namespace Xasin {
namespace NeoController {

uint32_t get_flashcycle_ticks() {
	return xTaskGetTickCount() / (100 / portTICK_PERIOD_MS);
}
uint32_t get_flashcycle_count() {
	return get_flashcycle_ticks() / 8;
}

IndicatorBulb::IndicatorBulb() :
		current(), deactivateAfter(0),
		target(), mode(OFF), flash_fill(8) {
}

IndicatorBulb& IndicatorBulb::operator =(const IndicatorBulb &top) {
	this->target = top.target;
	this->mode	 = top.mode;
	this->flash_fill = top.flash_fill;

	if(top.deactivateAfter != 0)
		this->deactivateAfter = xTaskGetTickCount() + top.deactivateAfter;
	else
		this->deactivateAfter = 0;

	return *this;
}
IndicatorBulb& IndicatorBulb::operator =(const bulb_config_t &config) {
	this->target = config.target;
	this->mode = config.mode;
	this->flash_fill = config.flash_fill;

	return *this;
}

void IndicatorBulb::set(Color target, bulb_mode_t mode, uint8_t fill, TickType_t deactivateTicks) {
	this->target = target;
	this->mode = mode;
	this->flash_fill = fill;

	deactivate_after(deactivateTicks);
}

Color IndicatorBulb::tick() {
	if((deactivateAfter != 0) && (deactivateAfter < xTaskGetTickCount())) {
		mode = OFF;
		deactivateAfter = 0;
	}

	auto bufferedTarget = target;
	bool onBuffer = false;

	switch(mode) {
	case OFF: current = Color(); break;

	case IDLE:
		current.merge_overlay(
				bufferedTarget.bMod((get_flashcycle_count()&1) ? 110 : 120)
				, 30);
	break;

	case HFLASH:
		onBuffer = ((get_flashcycle_ticks()&0b1111) < flash_fill);

		current.merge_overlay(
				bufferedTarget.bMod(onBuffer ? 160 : 90)
				, onBuffer ? 120 : 80);
	break;

	case FLASH:
		onBuffer = ((get_flashcycle_ticks()&0b111) < flash_fill/2);
		current.merge_overlay(
				bufferedTarget.bMod(onBuffer ? 180 : 80)
				, onBuffer ? 120 : 90);
	break;

	case DFLASH:
		onBuffer = ((get_flashcycle_ticks()&0b11) < flash_fill/4);
		current.merge_overlay(
				bufferedTarget.bMod(onBuffer ? 230 : 80)
				, onBuffer ? 180 : 130);
	break;

	case VAL_RISING:
		if((get_flashcycle_ticks()&0b111) == 0)
			current.merge_overlay(bufferedTarget.bMod(60), 100);
		else
			current.merge_overlay(bufferedTarget.bMod(130), 10);
	break;

	case VAL_FALLING:
		if((get_flashcycle_ticks()&0b111) == 0)
			current.merge_overlay(bufferedTarget.bMod(130), 100);
		else
			current.merge_overlay(bufferedTarget.bMod(60), 10);
	break;
	}


	return current;
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
