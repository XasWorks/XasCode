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

void AnimationElement::delete_copy_to(animation_value_id_t value) {
	copy_ops.erase(value);
}

float * AnimationElement::get_flt(animation_value_id_t val_num) {
	return nullptr;
}
Color * AnimationElement::get_color(uint8_t val_num) {
	return nullptr;
}

#define NEXT_SPACE() do { command = strpbrk(command, " ;"); if((command == nullptr) || (*command == ';')) return; if(*(++command) == '\0') return; } while(0)
void AnimationElement::set_flt(animation_value_id_t value, const char *command) {
	auto ptr = get_flt(value);

	if(ptr == nullptr)
		return;

	if(*command == 'J') {
		command += 1;
		*ptr = strtof(command, nullptr);
		NEXT_SPACE();
	}

	if(copy_ops.count(value) == 0)
		copy_ops[value] = {};
	copy_ops[value].to = ptr;

	auto &op = copy_ops[value];

	if(*command == 'V') {
		command += 1;

		op.pt2_speed = strtof(command, nullptr);
		NEXT_SPACE();
	}

	if(*command == 'S') {
		op.from_id = AnimationServer::decode_value_tgt(command);
		op.from = server.get_float_ptr(op.from_id);
		NEXT_SPACE();
	}

	op.add_offset = strtof(command, nullptr);
	NEXT_SPACE();
	op.mult_offset = strtof(command, nullptr);
	NEXT_SPACE();
	op.pt2_d = strtof(command, nullptr);
	NEXT_SPACE();
	op.pt2_t = strtof(command, nullptr);
}
void AnimationElement::set_flt(animation_value_id_t val_num, float new_val) {
	auto ptr = get_flt(val_num);
	if(ptr == nullptr)
		return;

	if(copy_ops.count(val_num) == 0)
		*ptr = new_val;
	else
		copy_ops[val_num].add_offset = new_val;
}
void AnimationElement::set_flt(animation_value_id_t val_num, animation_global_id_t source) {
	auto ptr = get_flt(val_num);
	if(ptr == nullptr)
		return;

	copy_ops[val_num].to = ptr;
	copy_ops[val_num].from_id = source;
	copy_ops[val_num].from = server.get_float_ptr(source);
}
void AnimationElement::set_flt_op(animation_value_id_t val_num, animation_copy_op op) {
	auto ptr = get_flt(val_num);
	if(ptr == nullptr)
		return;

	copy_ops[val_num] = op;
	copy_ops[val_num].to = ptr;
}

void AnimationElement::set_color(uint8_t value, const char *command) {
	auto ptr = get_color(value);

	if(ptr == nullptr)
		return;

	if(*command == 'J') {
		command += 1;
		*ptr = strtoll(command, nullptr, 16);
		NEXT_SPACE();
	}

	if(color_ops.count(value) == 0)
		color_ops[value] = {};

	color_ops[value].to = ptr;
	auto *op = &color_ops[value];

	if(*command == 'V') {
		command += 1;
		op->intermediate_color = strtoll(command, nullptr, 16);
		NEXT_SPACE();
	}

	op->target_color = strtoll(command, nullptr, 16);
	NEXT_SPACE();
	op->f1 = strtof(command, nullptr);
	NEXT_SPACE();
	op->f2 = strtof(command, nullptr);
}
void AnimationElement::set_color(uint8_t value, Color n_color) {
	auto ptr = get_color(value);
	if(ptr == nullptr)
		return;

	if(color_ops.count(value) == 0)
		*ptr = n_color;
	else
		color_ops[value].target_color = n_color;
}
void AnimationElement::set_color_op(uint8_t val_num, animation_color_op op) {
	auto ptr = get_color(val_num);
	if(ptr == nullptr)
		return;

	color_ops[val_num] = op;
	color_ops[val_num].to = ptr;
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
