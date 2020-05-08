/*
 * NumericElement.h
 *
 *  Created on: 15 Mar 2020
 *      Author: xasin
 */

#ifndef STM32F4_ANIMATION_NUMERICELEMENT_H_
#define STM32F4_ANIMATION_NUMERICELEMENT_H_

#include <Animation/AnimationElement.h>

#include <array>

namespace Xasin {

enum numeric_operator_t {
	TMR_PWM,
	TMR_SAWTOOTH,
	NUM_MAX,
};

struct pwm_cfg_t {
	float output;
	float duration;
	float phase_offset;
	float dutycycle;
	float min_val;
	float max_val;
};

union numeric_element_data_t {
	float start;
	pwm_cfg_t pwm_cfg;
};

class NumericElement: public AnimationElement {
public:
	numeric_element_data_t config;

	numeric_operator_t type;

public:

	NumericElement(AnimationServer &server, animation_id_t ID);

	float * get_flt(animation_value_id_t val);

	void tick(float delta_t);
};

} /* namespace Xasin */

#endif /* STM32F4_ANIMATION_NUMERICELEMENT_H_ */
