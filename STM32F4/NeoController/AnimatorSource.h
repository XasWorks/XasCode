/*
 * AnimatorSource.h
 *
 *  Created on: 3 Mar 2020
 *      Author: xasin
 */

#ifndef STM32F4_NEOCONTROLLER_ANIMATORSOURCE_H_
#define STM32F4_NEOCONTROLLER_ANIMATORSOURCE_H_

#include <stdint.h>

namespace Xasin {

union animation_id_t {
	struct {
		uint8_t set_id;
		uint8_t module_id;
	};
	uint16_t uniq_id;
};

struct animation_value_id_t {
	animation_id_t ID;
	uint8_t value;
};

class AnimatorSource {
protected:
	animation_id_t ID;

public:
	AnimatorSource();
	AnimatorSource(animation_id_t ID);

	static animation_value_id_t decode_value_target(const char *str);

	virtual float * get_float_value(uint8_t number);
	virtual void set_float_value(uint8_t number, const char *description);

	animation_id_t get_anim_id();
};

} /* namespace Xasin */

#endif /* STM32F4_NEOCONTROLLER_ANIMATORSOURCE_H_ */
