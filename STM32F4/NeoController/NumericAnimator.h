/*
 * NumericAnimator.h
 *
 *  Created on: 1 Mar 2020
 *      Author: xasin
 */

#ifndef STM32F4_NEOCONTROLLER_NUMERICANIMATOR_H_
#define STM32F4_NEOCONTROLLER_NUMERICANIMATOR_H_

#include <stdint.h>
#include <vector>

namespace Xasin {

enum animator_operator_t : uint16_t {
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
};

struct animator_step_t {
	animator_operator_t type;
	struct {
		bool in_a_const : 1;
		bool in_b_const : 1;
		bool out_const : 1;
	};

	union {
		const float * ptr;
		float var;
	} in_a;

	union {
		const float * ptr;
		float var;
	} in_b;

	union {
		float * ptr;
		float var;
	} out;
};

class NumericAnimator {
private:
	void exec_step(animator_step_t &step);

	float delta_t;

public:
	std::vector<animator_step_t> steps;

	NumericAnimator();

	void tick(float delta_t);

	animator_step_t * add_step(animator_operator_t type);

	NumericAnimator & set_a_p(const float *ptr);
	NumericAnimator & set_a_c(float var);
	NumericAnimator & set_b_p(const float *ptr);
	NumericAnimator & set_b_c(float var);
	NumericAnimator & set_o_p(float *ptr);
	NumericAnimator & set_o_c(float var);
};

} /* namespace Xasin */

#endif /* STM32F4_NEOCONTROLLER_NUMERICANIMATOR_H_ */
