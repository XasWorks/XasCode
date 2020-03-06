/*
 * NumericAnimator.cpp
 *
 *  Created on: 1 Mar 2020
 *      Author: xasin
 */

#include <NeoController/NumericAnimator.h>
#include <cmath>

namespace Xasin {

NumericAnimator::NumericAnimator() : steps() {
	steps.reserve(10);
}

#define STEP_IO(designator) ((step.designator ## _const) ? step.designator.var : (*step.designator.ptr))

void NumericAnimator::exec_step(animator_step_t &step) {
	switch(step.type) {
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

void NumericAnimator::tick(float delta_t) {
	this->delta_t = delta_t;

	for(auto &s : steps)
		exec_step(s);
}

animator_step_t * NumericAnimator::add_step(animator_operator_t s) {
	animator_step_t tmp = {};
	tmp.type = s;
	tmp.in_a_const = true;
	tmp.in_b_const = true;
	tmp.out_const = true;

	tmp.in_a.var = 0;
	tmp.in_b.var = 0;
	tmp.out.var = 0;

	steps.push_back(tmp);

	return (--steps.end()).base();
}

NumericAnimator & NumericAnimator::set_a_p(const float *ptr) {
	(--steps.end())->in_a_const = false;
	(--steps.end())->in_a.ptr = ptr;

	return *this;
}
NumericAnimator & NumericAnimator::set_b_p(const float *ptr) {
	(--steps.end())->in_b_const = false;
	(--steps.end())->in_b.ptr = ptr;

	return *this;
}
NumericAnimator & NumericAnimator::set_o_p(float *ptr) {
	(--steps.end())->out_const = false;
	(--steps.end())->out.ptr = ptr;

	return *this;
}

NumericAnimator & NumericAnimator::set_a_c(float var) {
	(--steps.end())->in_a_const = true;
	(--steps.end())->in_a.var = var;

	return *this;
}
NumericAnimator & NumericAnimator::set_b_c(float var) {
	(--steps.end())->in_b_const = true;
	(--steps.end())->in_b.var = var;

	return *this;
}
NumericAnimator &  NumericAnimator::set_o_c(float var) {
	(--steps.end())->out_const = true;
	(--steps.end())->out.var = var;

	return *this;
}


} /* namespace Xasin */
