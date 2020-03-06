/*
 * AnimationStep.h
 *
 *  Created on: 3 Mar 2020
 *      Author: xasin
 */

#ifndef STM32F4_NEOCONTROLLER_ANIMATIONSTEP_H_
#define STM32F4_NEOCONTROLLER_ANIMATIONSTEP_H_

#include "AnimatorSource.h"

#include <array>
#include <string>

namespace Xasin {

enum animator_operator_t {
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
	MAX
};

class AnimationStep : public AnimatorSource {
private:
	bool needs_relink;

	struct data_io_t {
		bool use_ptr;
		union {
			float *f_ptr;
			float  var;
		};
		animation_value_id_t target_value;
	};

	animator_operator_t calc_type;

	std::array<data_io_t, 3> data_ios;

	void select_type(const char *type_ptr);
public:
	AnimationStep();
	AnimationStep(animation_id_t ID);
	AnimationStep(const char * descriptor);

	float *get_float_value(uint8_t number);
	void set_float_value(uint8_t number, const char *description);

	void tick(float delta_t);
};

} /* namespace Xasin */

#endif /* STM32F4_NEOCONTROLLER_ANIMATIONSTEP_H_ */
