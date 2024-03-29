/*
 * IndicatorBulb.h
 *
 *  Created on: 16 Apr 2019
 *      Author: xasin
 */

#ifndef MAIN_CORE_INDICATORBULB_H_
#define MAIN_CORE_INDICATORBULB_H_

#include "xnm/neocontroller.h"

namespace XNM {
namespace Neo {

enum bulb_mode_t {
	OFF,
	IDLE,
	RUNNING,
	ON,
	HFLASH,
	FLASH,
	DFLASH,
	VAL_RISING,
	VAL_FALLING,
};

struct bulb_config_t {
	bulb_mode_t mode;
	Color target;
	uint8_t flash_fill;
};

namespace Bulb {

static const bulb_config_t OFF = {bulb_mode_t::OFF, 0, 4};
static const bulb_config_t OK  = {bulb_mode_t::IDLE, Material::GREEN, 4};
static const bulb_config_t ATTENTION = {bulb_mode_t::FLASH, Material::AMBER, 8};

}

class IndicatorBulb {
protected:
	Color current;
	TickType_t deactivateAfter;

	uint8_t last_brightness;
	uint8_t current_brightness;

public:
	Color target;

	bulb_mode_t mode;
	uint8_t flash_fill;

	IndicatorBulb();
	IndicatorBulb& operator=(const bulb_config_t &config);

	void set(Color target, bulb_mode_t mode, TickType_t deactivateTicks = 0, uint8_t fill = 0);


	uint8_t pattern_brightness();

	Color color_tick();
	int   switch_tick();

	Color get_color();

	void deactivate_after(TickType_t ticks);
};

}
}

#endif /* MAIN_CORE_INDICATORBULB_H_ */
