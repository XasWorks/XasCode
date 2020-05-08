/*
 * AnimationElement.cpp
 *
 *  Created on: 15 Mar 2020
 *      Author: xasin
 */

#include <Animation/AnimationElement.h>

#include <string.h>
#include <math.h>
#include <stdlib.h>

int VIEWER_animation_count = 0;

namespace Xasin {

led_coord_t const * AnimationElement::led_coordinates = nullptr;

float AnimationElement::calc_coords(led_coord_t led, led_coord_t coords) {
	float sum = 0;

	auto led_c = reinterpret_cast<const float*>(&led);
	auto fact_c = reinterpret_cast<const float*>(&coords);

	for(int i=0; i<sizeof(led_coord_t) / sizeof(float); i++) {
		if(fact_c[i] == 0)
			continue;
		if(isnan(led_c[i]))
			return 0;

		sum = fmaf(led_c[i], fact_c[i], sum);
	}

	return sum;
}

float AnimationElement::calc_coords(int led, led_coord_t coords) {
	return calc_coords(led_coordinates[led], coords);
}

AnimationElement::AnimationElement(AnimationServer &server, animation_id_t ID, int z)
	:	draw_z(z),
		server(server), ID(ID) {

	server.delete_animation(ID);
	server.insert_pointer(this);

	VIEWER_animation_count++;
}

AnimationElement::~AnimationElement() {
	server.force_relink();
	server.remove_pointer(this);

	VIEWER_animation_count--;
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
