/*
 * Eyes.cpp
 *
 *  Created on: 17 May 2020
 *      Author: xasin
 */

#include <Animation/Eyes.h>
#include <math.h>

extern const uint8_t console_font[];

namespace Xasin {
namespace Animation {

Eyes::Eyes(AnimationServer &server, animation_id_t ID, Layer &target_layer)
	: AnimationElement(server, ID),
	  target_layer(target_layer), offset(30), outer_color(0x550000), inner_color(0),
	  top_rest(-3), bottom_rest(2), eye_close_factor(0),
	  iris_x(2), iris_y(2),
	  disp_string("Booting..."), scroll_speed(3), scroll_pos(0) {
}

float *Eyes::get_flt(animation_value_id_t val) {
	switch(val) {
	case 0x000:
		return &scroll_pos;
	case 0x001:
		return &scroll_speed;
	case 0x010:
		return &iris_x;
	case 0x011:
		return &iris_y;
	case 0x012:
		return &top_rest;
	case 0x013:
		return &bottom_rest;
	case 0x014:
		return &eye_close_factor;

	default: return nullptr;
	}
}

Color * Eyes::get_color(uint8_t val) {
	switch(val) {
	default:
		return nullptr;
	case 0:
		return &outer_color;
	case 1:
		return &inner_color;
	}
}

void Eyes::draw_color(int x, int y, const Color &color) {
	if(x < 0 || x >= 8 || y < 0 || y >= 8)
		return;

	target_layer[offset + x*8 + y].merge_overlay(color);
}
void Eyes::draw_buffer(const uint8_t *start, int start_x, int start_y, const Color &color) {
	const uint8_t *end = start + 8;

	start_y += 7;

	while(start != end) {
		if(start_y < 8 && start_y >= 0) {
			uint8_t c_row = *start;

			for(int i=start_x+7; i>=start_x; i--) {
				if(c_row & 1)
					draw_color(i, start_y, color);
				c_row >>= 1;
			}
		}

		start_y--;
		start++;
	}
}

void Eyes::mask_buffer(uint8_t *buffer, const uint8_t *mask, int mask_offset, bool mask_dir) {
	for(uint8_t i=0; i<8; i++) {
		uint8_t mask_row;

		if(i - mask_offset >= 8)
			mask_row = mask_dir ? 0xFF : 0;
		else if(i - mask_offset >= 0)
			mask_row = mask[i-mask_offset];
		else
			mask_row = mask_dir ? 0 : 0xFF;

		buffer[i] &= mask_row;
	}
}

void Eyes::draw_eye() {
	uint8_t eye[] = { 0x0, 0x1c, 0x7e, 0x7f, 0xff, 0xff, 0xfe, 0x0 };

	// Remove the top "closure" buffer for blinking and squinting
	const uint8_t top_eyelid[] = { 0x0, 0x0, 0x0, 0x0, 0x0, 0x1e, 0x7f, 0x7f };
	const uint8_t lower_eyelid[] = { 0xff, 0xff, 0xff, 0xff, 0xf0, 0xc0, 0x80, 0x0 };

	float local_factor = fmaxf(0, eye_close_factor);

	mask_buffer(eye, top_eyelid, roundf(top_rest * (1-local_factor)), true);
	mask_buffer(eye, lower_eyelid, roundf(bottom_rest * (1-local_factor)), false);

	draw_buffer(eye, 0, 0, outer_color);

	for(int y = iris_y; y < 8; y++) {
		draw_color(iris_x, y, inner_color);
	}

}

void Eyes::tick(float delta_t) {
	eye_close_factor =  2 - fabsf(fmodf(server.get_synch_time(), 6) - 3) * 15;

	if(disp_string.size() != 0) {
		scroll_pos += delta_t * scroll_speed;

		int int_scroll    = floorf(scroll_pos);
		int scroll_offset = floorf(fmodf(scroll_pos*6, 6));

		if((int_scroll >= 0) && (int_scroll < disp_string.size()))
			draw_buffer(console_font + 8*disp_string[int_scroll], -scroll_offset, 0, outer_color);
		if((int_scroll+1 >= 0) && (int_scroll+1 < disp_string.size()))
			draw_buffer(console_font + 8*disp_string[int_scroll+1], -scroll_offset + 6, 0, outer_color);

		if(int_scroll > disp_string.size()) {
			disp_string.clear();
		}
	}
	else {
		draw_eye();
	}
}

}
} /* namespace Xasin */
