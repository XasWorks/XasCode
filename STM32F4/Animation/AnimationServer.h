/*
 * AnimatorServer.h
 *
 *  Created on: 15 Mar 2020
 *      Author: xasin
 */

#ifndef STM32F4_NEOCONTROLLER_ANIMATORSERVER_H_
#define STM32F4_NEOCONTROLLER_ANIMATORSERVER_H_

#include <stdint.h>
#include <map>

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

struct animation_flt_val_t {
	animation_value_id_t link;
	float value;
	float * copy_ptr;
};

class AnimationElement;

class AnimationServer {
protected:
	friend AnimationElement;

	std::map<uint16_t, AnimationElement *> animations;

	bool needs_relink;

public:
	AnimationServer();

	void force_relink();

	AnimationElement * get_animation(animation_id_t id);

	animation_flt_val_t * get_float_value(animation_value_id_t id);
	float * get_float_ptr(animation_value_id_t id);

	void  tick(float delta_t);

	static animation_value_id_t decode_value_tgt(const char *str);
};

} /* namespace Xasin */

#endif /* STM32F4_NEOCONTROLLER_ANIMATORSERVER_H_ */
