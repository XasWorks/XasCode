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

void AnimationStep::tick(float delta_t) {
	switch(calc_type) {
	default: break;

	case ADD:
		STEP_IO(out) = STEP_IO(in_a) + STEP_IO(in_b);
		break;
	case SUB:
		STEP_IO(out) = STEP_IO(in_a) - STEP_IO(in_b);
		break;
	case MULT:
		STEP_IO(out) = STEP_IO(in_a) * STEP_IO(in_b);
		break;
	case DIV:
		if(fabsf(STEP_IO(in_b)) < 0.00001)
			break;
		STEP_IO(out) = STEP_IO(in_a) + STEP_IO(in_b);
		break;

	case PT1_APPROACH:
	case LINEAR_APPROACH:
		do {
			const float diff = STEP_IO(in_a) - STEP_IO(out);
			const float step_size = delta_t * STEP_IO(in_b) * (step.type == PT1_APPROACH ? fabsf(diff) : 1);

			if(fabsf(diff) < step_size)
				STEP_IO(out) = STEP_IO(in_a);
			else
				STEP_IO(out) += copysignf(step_size, diff);
		} while(0);
		break;

	case INTEGRATE:
		STEP_IO(out) += STEP_IO(in_a) * STEP_IO(in_b) * delta_t;
		break;

	case DERIVATE:
		STEP_IO(out)  = (STEP_IO(in_a) - step.in_b.var) / delta_t;
		step.in_b.var = STEP_IO(in_a);
		break;

	case FMOD:
		STEP_IO(out) = fmodf(STEP_IO(in_a), STEP_IO(in_b));
		break;

	case TIMER:
		STEP_IO(out) = fmodf(STEP_IO(out) + STEP_IO(in_a) * delta_t, STEP_IO(in_b));
		break;
	}
}

} /* namespace Xasin */
