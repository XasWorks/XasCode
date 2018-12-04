/*
 * NeoController.h
 *
 *  Created on: 14 Sep 2018
 *      Author: xasin
 */

#ifndef MAIN_NEOCONTROLLER_H_
#define MAIN_NEOCONTROLLER_H_

#include "driver/rmt.h"
#include "esp_pm.h"
#include "esp32/pm.h"

#include "freertos/task.h"

#include "Color.h"
#include "Layer.h"

namespace Peripheral {

class NeoController {
public:
	const uint8_t length;

	Layer colors;
	Layer nextColors;

private:
	const gpio_num_t pinNo;
	const rmt_channel_t channel;

	Color::ColorData *rawColors;

	esp_pm_lock_handle_t powerLock;

public:
	NeoController(gpio_num_t pin, rmt_channel_t channel, uint8_t length);

	void update();

	void clear();
	void fill(Color color);
	Color& operator [](int id);
	Color * get(int id);

	void apply();

	void fadeTransition(uint32_t duration);
	void swipeTransition(uint32_t duration, bool desc = false);
};

} /* namespace Touch */

#endif /* MAIN_NEOCONTROLLER_H_ */
