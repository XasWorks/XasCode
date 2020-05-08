/*
 * NumericElement.cpp
 *
 *  Created on: 15 Mar 2020
 *      Author: xasin
 */

#include <Animation/NumericElement.h>

#include <math.h>

namespace Xasin {

NumericElement::NumericElement(AnimationServer &server, animation_id_t ID)
	: AnimationElement(server, ID),
	  config(), type(NUM_MAX) {

}

float * NumericElement::get_flt(animation_value_id_t val) {
	if(val >= (sizeof(config) / sizeof(float)))
		return nullptr;

	return (&config.start + val);
}

void NumericElement::tick(float delta_t) {
	switch(type) {
	default: break;

	case TMR_PWM: do {
		auto const &pwm_cfg = config.pwm_cfg;

		float tmr_time = server.get_synch_time() / pwm_cfg.duration;
		tmr_time += pwm_cfg.phase_offset;
		tmr_time = fmodf(tmr_time, 1);

		if(tmr_time <= pwm_cfg.dutycycle)
			config.start = pwm_cfg.min_val;
		else
			config.start = pwm_cfg.max_val;
	} while(0);
	}
}

} /* namespace Xasin */
