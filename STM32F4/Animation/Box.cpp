/*
 * Box.cpp
 *
 *  Created on: 23 Mar 2020
 *      Author: xasin
 */

#include <Animation/Box.h>

#include <math.h>

namespace Xasin {
namespace Animation {

Box::Box(AnimationServer &server, animation_id_t ID, Layer &tgt_layer, int z)
	: AnimationElement(server, ID, z), target(tgt_layer),
	  x_coord(), y_coord(), center(),
	  rotation(0), up(0), down(0), left(0), right(0),
	  draw_color() {
}

float * Box::get_flt(animation_value_id_t val) {
	const uint16_t chop_val = val & 0x0FFF;
	const uint16_t top_val  = (val & 0xF000) >> 12;

	switch(top_val) {
	case 0x0:
		if((chop_val >= 0) && (chop_val <= 4)) {
			float * const ptr[] = { &rotation, &up, &down, &left, &right };
			return ptr[chop_val];
		}
		return nullptr;

	case 0xA:
		if(chop_val == 0)
			return &(draw_color.alpha);
		return nullptr;

	case 0xC:
		if((chop_val & 0xFF) < LED_COORD_COUNT) {
			const int coord_num = chop_val >> 8;
			if(coord_num >= 3)
				return nullptr;
			const int axis_num = chop_val & 0xFF;
			led_coord_t * const ptr[] = { &x_coord, &y_coord, &center };
			return &(reinterpret_cast<float *>(ptr[coord_num])[axis_num]);
		}
		return nullptr;

	default: return nullptr;
	}
}

Color * Box::get_color(uint8_t val) {
	if(val == 0)
		return &draw_color;

	return nullptr;
}

void Box::tick(float _delta_t) {
	const float rf_sin = sinf(rotation);
	const float rf_cos = cosf(rotation);

	const float true_center_x = calc_coords(center, x_coord);
	const float true_center_y = calc_coords(center, y_coord);

	for(int i=0; i<ANIM_LED_COUNT; i++) {
		const float led_x = calc_coords(i, x_coord) - true_center_x;
		const float led_y = calc_coords(i, y_coord) - true_center_y;

		const float r_led_x = rf_cos * led_x + rf_sin * led_y;
		const float r_led_y = rf_cos * led_y - rf_sin * led_x;

		if(r_led_x > right)
			continue;
		if(r_led_x < -left)
			continue;
		if(r_led_y > up)
			continue;
		if(r_led_y < -down)
			continue;

		target[i].merge_overlay(draw_color);
	}
}

} /* namespace Animation */
} /* namespace Xasin */
