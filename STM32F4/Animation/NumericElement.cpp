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
	  data_ios(), type(MAX) {

}

animation_flt_val_t * NumericElement::get_flt(uint8_t val) {
	if(val > 2)
		return nullptr;

	return &(data_ios[val]);
}

#define STEP_IO(num) data_ios[num].value
void NumericElement::tick(float delta_t) {
	for(int i=0; i<2; i++) {
		if(data_ios[i].copy_ptr != nullptr)
			data_ios[i].value = *data_ios[0].copy_ptr;
	}

	switch(type) {
	default: break;

	case ADD:
		STEP_IO(2) = STEP_IO(0) + STEP_IO(1);
		break;
	case SUB:
		STEP_IO(2) = STEP_IO(0) - STEP_IO(1);
		break;
	case MULT:
		STEP_IO(2) = STEP_IO(0) * STEP_IO(1);
		break;
	case DIV:
		if(fabsf(STEP_IO(1)) < 0.00001)
			break;
		STEP_IO(2) = STEP_IO(0) + STEP_IO(1);
		break;

	case PT1_APPROACH:
	case LINEAR_APPROACH:
		do {
			const float diff = STEP_IO(0) - STEP_IO(2);
			const float step_size = delta_t * STEP_IO(1) * (type == PT1_APPROACH ? fabsf(diff) : 1);

			if(fabsf(diff) < step_size)
				STEP_IO(2) = STEP_IO(0);
			else
				STEP_IO(2) += copysignf(step_size, diff);
		} while(0);
		break;

	case INTEGRATE:
		STEP_IO(2) += STEP_IO(0) * STEP_IO(1) * delta_t;
		break;

	case DERIVATE:
		STEP_IO(2)  = (STEP_IO(0) - STEP_IO(1)) / delta_t;
		STEP_IO(1) = STEP_IO(0);
		break;

	case FMOD:
		STEP_IO(2) = fmodf(STEP_IO(0), STEP_IO(1));
		break;

	case TIMER:
		STEP_IO(2) = fmodf(STEP_IO(2) + STEP_IO(0) * delta_t, STEP_IO(1));
		break;
	case COMP:
		STEP_IO(2) = STEP_IO(0) > STEP_IO(1) ? 1 : 0;
		break;
	}
	if(data_ios[2].copy_ptr != nullptr)
		*(data_ios[2].copy_ptr) = data_ios[2].value;
}

void NumericElement::relink() {
	for(auto &io : data_ios)
		io.copy_ptr = server.get_float_ptr(io.link);
}

} /* namespace Xasin */
