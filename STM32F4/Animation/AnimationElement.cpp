/*
 * AnimationElement.cpp
 *
 *  Created on: 15 Mar 2020
 *      Author: xasin
 */

#include <Animation/AnimationElement.h>

#include <cstring>

namespace Xasin {

AnimationElement::AnimationElement(AnimationServer &server, animation_id_t ID)
	: server(server), ID(ID) {

	server.animations[ID.uniq_id] = this;
	server.force_relink();
}

AnimationElement::~AnimationElement() {
	if(server.animations[ID.uniq_id] != this)
		return;

	server.animations.erase(ID.uniq_id);

	server.force_relink();
}

animation_flt_val_t * AnimationElement::get_flt(uint8_t val_num) {
	return nullptr;
}
void AnimationElement::set_flt(uint8_t val_num, const char *description) {
	auto val = get_flt(val_num);
	if(val == nullptr)
		return;

	if(strspn(description, " 0123456789.-+") == strlen(description)) {
		val->copy_ptr = nullptr;
		val->value = atof(description);
	}
	else {
		val->copy_ptr = nullptr;
		val->link = AnimationServer::decode_value_tgt(description);
		server.force_relink();
	}
}

void AnimationElement::tick(float delta_t) {
}
void AnimationElement::relink() {
}

} /* namespace Xasin */
