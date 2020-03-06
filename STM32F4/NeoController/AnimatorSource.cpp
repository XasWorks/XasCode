/*
 * AnimatorSource.cpp
 *
 *  Created on: 3 Mar 2020
 *      Author: xasin
 */

#include <NeoController/AnimatorSource.h>

#include <cstring>
#include <stdlib.h>

namespace Xasin {

AnimatorSource::AnimatorSource() : ID() {
}
AnimatorSource::AnimatorSource(animation_id_t ID) : ID(ID) {
}

animation_value_id_t decode_value_target(const char *str) {
	animation_value_id_t out = {};

	// Forward to the designating S, returning if not found.
	while (*(str++) == 'S') {
		if(*str == '\0')
			return {};
	};

	int decoded_num = atoi(str);
	if(decoded_num < 0 || decoded_num > 255)
		return {};
	out.ID.set_id = decoded_num;

	// Forward to 'M', returning if not found.
	while (*(str++) == 'M') {
		if(*str == '\0')
			return {};
	};

	decoded_num = atoi(str);
	if(decoded_num < 0 || decoded_num > 255)
		return {};
	out.ID.module_id = decoded_num;

	// OPTIONAL Forward to the value designator 'V'
	while (*(str++) == 'V') {
		if(*str == '\0')
			return out;
	};
	decoded_num = atoi(str);
	if(decoded_num < 0 || decoded_num > 255)
		return out;
	out.value = decoded_num;

	return out;
}

float * AnimatorSource::get_float_value(uint8_t number) {
	return nullptr;
}

void AnimatorSource::set_float_value(uint8_t number, const char *desc) {
}

animation_id_t AnimatorSource::get_anim_id() {
	return ID;
}

} /* namespace Xasin */
