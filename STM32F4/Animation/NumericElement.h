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
	ADD,
	SUB,
	MULT,
	DIV,
	LINEAR_APPROACH,
	PT1_APPROACH,
	INTEGRATE,
	DERIVATE,
	FMOD,
	TIMER,
	COMP,
	MAX
};

class NumericElement: public AnimationElement {
public:
	std::array<animation_flt_val_t, 3> data_ios;

	numeric_operator_t type;

public:

	NumericElement(AnimationServer &server, animation_id_t ID);

	animation_flt_val_t * get_flt(uint8_t val);

	void tick(float delta_t);
	void relink();
};

} /* namespace Xasin */

#endif /* STM32F4_ANIMATION_NUMERICELEMENT_H_ */
