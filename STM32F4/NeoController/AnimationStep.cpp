/*
 * AnimationStep.cpp
 *
 *  Created on: 3 Mar 2020
 *      Author: xasin
 */

#include <NeoController/AnimationStep.h>

#include <cstring>
#include <cmath>

namespace Xasin {

AnimationStep::AnimationStep()
	: calc_type(ADD), data_ios() {

}

AnimationStep::AnimationStep(animation_id_t ID)
	: AnimatorSource(ID) {
}

AnimationStep::AnimationStep(const char *descriptor) {
	auto first_id = AnimatorSource::decode_value_target(descriptor);
	ID.uniq_id = first_id.ID.uniq_id;

	if(ID.uniq_id == 0)
		return;

	const char * process_ptr = strpbrk(descriptor, ":");
	if(process_ptr++ == nullptr)
		return;
	select_type(process_ptr);

	for(int i=0; i<3; i++) {
		process_ptr = strpbrk(process_ptr, ",");
		if(process_ptr++ == nullptr)
			return;
		set_float_value(i, process_ptr);
	}
}

void AnimationStep::select_type(const char *type_ptr) {
	const char * const type_designators[] = {
		"+",
		"-",
		"*",
		"/",
		"LIN",
		"PT1",
		"INT",
		"DRV",
		"MOD",
		"TMR"
	};

	for(int i=0; i<MAX; i++) {
		if(strstr(type_ptr, type_designators[i]) != nullptr) {
			calc_type = static_cast<animator_operator_t>(i);
			return;
		}
	}

	calc_type = MAX;
}

void AnimationStep::set_float_value(uint8_t number, const char *description) {
	if(number >= 3)
		return;

	data_ios[number].target_value = {};

	if(strpbrk(description, "S")) {
		data_ios[number].use_ptr = true;
		data_ios[number].f_ptr = nullptr;

		data_ios[number].target_value = decode_value_target(description);

		needs_relink = true;
	}
	else {
		data_ios[number].use_ptr = false;
		data_ios[number].var = atof(description);
	}
}

#define STEP_IO(designator) ((data_ios[designator].use_ptr) ? *data_ios[designator].f_ptr : (data_ios[designator].var))
void AnimationStep::tick(float delta_t) {
	switch(calc_type) {
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
			const float step_size = delta_t * STEP_IO(1) * (calc_type == PT1_APPROACH ? fabsf(diff) : 1);

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
	}
}

} /* namespace Xasin */
