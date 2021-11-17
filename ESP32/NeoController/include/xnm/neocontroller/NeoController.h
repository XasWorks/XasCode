/*
 * NeoController.h
 *
 *  Created on: 14 Sep 2018
 *      Author: xasin
 */

#ifndef MAIN_NEOCONTROLLER_H_
#define MAIN_NEOCONTROLLER_H_

#include "Color.h"
#include "Layer.h"
#include "driver/rmt.h"
#include "esp_pm.h"
#include "esp32/pm.h"

#include "freertos/task.h"


namespace XNM {
namespace Neo {

class NeoController {
public:
	const uint8_t length;

	Layer colors;
	Layer nextColors;

private:
	bool is_initialized;

	const gpio_num_t pinNo;
	const rmt_channel_t channel;

	uint8_t *rawColors;

	esp_pm_lock_handle_t powerLock;

public:
	NeoController(gpio_num_t pin, rmt_channel_t channel, uint8_t length);

	/**
	 * @brief Initialize the NeoController module
	 * 
	 * @note It is suggested to init the NeoController ISR from a different core than Core0
	 * 	Core0 should be reserved for networking functions, with Core 1 being for critical
	 * 	and RMT options.
	 */
	void init();

	void update();

	void clear();
	void fill(Color color);
	Color& operator [](int id);
	Color * get(int id);

	void apply();

	void fadeTransition(uint32_t duration);
	void swipeTransition(uint32_t duration, bool desc = false);
};

}
} /* namespace Touch */

#endif /* MAIN_NEOCONTROLLER_H_ */
