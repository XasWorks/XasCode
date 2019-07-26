/*
 * OneShot125.h
 *
 *  Created on: 21 Jul 2019
 *      Author: xasin
 */

#ifndef ESP32_ONESHOT125_ONESHOT125_H_
#define ESP32_ONESHOT125_ONESHOT125_H_

#include "driver/gpio.h"
#include "driver/ledc.h"

#include <vector>

namespace Xasin {

class OneShot125 {
private:
	std::vector<float> motor_tgt_pwr;

	void init_channel(uint8_t pin, uint8_t led_channel);
	void set_channel_ns(uint8_t channel, uint32_t nanos);

public:
	const uint8_t num_channels;

	const ledc_timer_t one_shot_timer;
	const uint8_t led_start_channel;

	const uint8_t gpio_start;

	OneShot125(uint8_t channel_count, ledc_timer_t timer, uint8_t chStart, uint8_t gpio_start);
	virtual ~OneShot125();

	void init();

	void set_motor_power(uint8_t id, float val);

	float get_real_power(uint8_t id);
	float get_tgt_power(uint8_t id);
};

} /* namespace Xasin */

#endif /* ESP32_ONESHOT125_ONESHOT125_H_ */
