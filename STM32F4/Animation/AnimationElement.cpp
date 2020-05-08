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
		delete_after(0),
		copy_ops(), color_ops(),
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
Color * AnimationElement::get_color(uint8_t val_num) {
	return nullptr;
}
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

void AnimationElement::copy_op_tick(float delta_t) {
	for(auto c_it = copy_ops.begin(); c_it != copy_ops.end(); c_it++) {
		animation_copy_op &c = (*c_it).second;

		float tgt_val = (c.from == nullptr) ? 0 : *(c.from);
		tgt_val += c.add_offset;

		if(c.mult_offset != 0)
			tgt_val *= c.mult_offset;

		if(c.pt2_d < 0.01)
			(*c.to) = tgt_val;
		else if(c.pt2_t < 0.01) {
			(*c.to) += delta_t * (tgt_val - (*c.to)) / c.pt2_d;
		}
		else {
			float target_speed = (tgt_val - (*c.to)) / c.pt2_d;
			c.pt2_speed += delta_t * (target_speed - c.pt2_speed) / c.pt2_t;
			(*c.to) += delta_t * c.pt2_speed;
		}
	}
}

void AnimationElement::color_op_tick(float delta_t) {
	for(auto &c_it : color_ops) {
		auto &c_op = c_it.second;
		if(c_op.f2 == 0)
			(*c_op.to).merge_transition(c_op.target_color, delta_t * c_op.f1);
		else {
			c_op.intermediate_color.merge_transition(c_op.target_color, delta_t * c_op.f1);
			(*c_op.to).merge_transition(c_op.intermediate_color, delta_t * c_op.f2);
		}
	}
}

void AnimationElement::copy_tick(float delta_t) {
	copy_op_tick(delta_t);
	color_op_tick(delta_t);
}
void AnimationElement::relink() {
	for(auto &c : copy_ops) {
		c.second.from = server.get_float_ptr(c.second.from_id);
	}
}

} /* namespace Xasin */
