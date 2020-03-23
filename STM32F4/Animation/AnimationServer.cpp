/*HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_8);
 * AnimationServer.cpp
 *
 *  Created on: 15 Mar 2020
 *      Author: xasin
 */

#include <Animation/AnimationServer.h>
#include <Animation/AnimationElement.h>

#include <vector>

namespace Xasin {

AnimationServer::AnimationServer()
	: animations(),
	  needs_relink(false), needs_deletion(false),
	  synch_time(0) {
}

void AnimationServer::force_relink() {
	needs_relink = true;
}

AnimationElement * AnimationServer::get_animation(animation_id_t id) {
	if(animations.count(id.uniq_id) == 0)
		return nullptr;

	return animations[id.uniq_id];
}

animation_flt_val_t * AnimationServer::get_float_value(animation_value_id_t id) {
	auto anim = get_animation(id.ID);
	if(anim == nullptr)
		return nullptr;

	return anim->get_flt(id.value);
}
float * AnimationServer::get_float_ptr(animation_value_id_t id) {
	auto val = get_float_value(id);
	if(val == nullptr)
		return nullptr;

	return &(val->value);
}

void AnimationServer::tick(float delta_t) {
	synch_time += delta_t;

	if(needs_deletion) {
		std::vector<AnimationElement *> to_delete = {};

		for(auto element : animations) {
			if(element.second->delete_after == 0)
				continue;
			if(element.second->delete_after < synch_time)
				to_delete.push_back(element.second);
		}

		for(auto ptr : to_delete)
			delete ptr;

		needs_deletion = false;
	}

	if(needs_relink) {
		for(auto element : animations)
			element.second->relink();
		needs_relink = false;
	}

	for(auto element : animations) {
		if(element.second->delete_after < synch_time)
			needs_deletion = true;

		element.second->tick(delta_t);
	}
}

float AnimationServer::get_synch_time() {
	return synch_time;
}

animation_value_id_t AnimationServer::decode_value_tgt(const char *str) {
	animation_value_id_t out = {};

	// Forward to the designating S, returning if not found.
	while (*(str++) != 'S') {
		if(*str == '\0')
			return {};
	};

	int decoded_num = atoi(str);
	if(decoded_num < 0 || decoded_num > 255)
		return {};
	out.ID.set_id = decoded_num;

	// Forward to 'M', returning if not found.
	while (*(str++) != 'M') {
		if(*str == '\0')
			return {};
	};

	decoded_num = atoi(str);
	if(decoded_num < 0 || decoded_num > 255)
		return {};
	out.ID.module_id = decoded_num;

	// OPTIONAL Forward to the value designator 'V'
	while (*(str++) != 'V') {
		if(*str == '\0')
			return out;
	};
	decoded_num = atoi(str);
	if(decoded_num < 0 || decoded_num > 255)
		return out;
	out.value = decoded_num;

	return out;
}

} /* namespace Xasin */
