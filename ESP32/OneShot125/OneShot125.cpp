/*
 * OneShot125.cpp
 *
 *  Created on: 21 Jul 2019
 *      Author: xasin
 */

#include "xasin/OneShot125.h"

#define LOG_LOCAL_LEVEL ESP_LOG_INFO
#include "esp_log.h"

#define LOGTAG "OS125"

namespace Xasin {

OneShot125::OneShot125(uint8_t channel_count, ledc_timer_t timer, uint8_t chStart, uint8_t gpio_start)
	: motor_tgt_pwr(channel_count),
	  num_channels(channel_count),
	  one_shot_timer(timer), led_start_channel(chStart),
	  gpio_start(gpio_start) {
}

void OneShot125::init() {
	ledc_timer_config_t ledCFG = {};
	ledCFG.speed_mode = LEDC_LOW_SPEED_MODE;
	ledCFG.duty_resolution = LEDC_TIMER_15_BIT;
	ledCFG.timer_num = one_shot_timer;
	ledCFG.freq_hz   = 1000;

	ledc_timer_config(&ledCFG);

	for(uint8_t i=0; i<num_channels; i++)
		init_channel(gpio_start+i, led_start_channel+i);

	ledc_fade_func_install(0);

	ESP_LOGI(LOGTAG, "Channels initialised");
}

void OneShot125::init_channel(uint8_t pin, uint8_t led_channel) {
	gpio_reset_pin(static_cast<gpio_num_t>(pin));

	ledc_channel_config_t ledChannel = {};
	ledChannel.gpio_num = pin;
	ledChannel.speed_mode = LEDC_LOW_SPEED_MODE;
	ledChannel.channel = static_cast<ledc_channel_t>(led_start_channel + led_channel);
	ledChannel.intr_type = LEDC_INTR_DISABLE;
	ledChannel.timer_sel = one_shot_timer;
	ledChannel.duty = 0;
	ledc_channel_config(&ledChannel);
}
void OneShot125::set_channel_ns(uint8_t channel, uint32_t nanos) {
	auto ledChannel = static_cast<ledc_channel_t>(led_start_channel + channel);

	ledc_set_duty(LEDC_LOW_SPEED_MODE, ledChannel, (nanos * 32767) / 10000);
	ledc_update_duty(LEDC_LOW_SPEED_MODE, ledChannel);
}


OneShot125::~OneShot125() {
	for(uint8_t i=0; i<num_channels; i++)
		gpio_reset_pin(static_cast<gpio_num_t>(gpio_start+i));
}

void OneShot125::set_motor_power(uint8_t id, float val) {
	if(id >= num_channels)
		return;

	if(val > 1)
		val = 1;
	if(val < 0)
		val = 0;

	motor_tgt_pwr[id] = val;

	set_channel_ns(id, 1250 + 1250*val);
}

float OneShot125::get_real_power(uint8_t id) {
	if(id >= num_channels)
		return 0;

	return motor_tgt_pwr[id];
}

} /* namespace Xasin */
