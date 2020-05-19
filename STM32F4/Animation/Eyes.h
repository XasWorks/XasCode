/*
 * Eyes.h
 *
 *  Created on: 17 May 2020
 *      Author: xasin
 */

#ifndef STM32F4_ANIMATION_EYES_H_
#define STM32F4_ANIMATION_EYES_H_

#include <Animation/AnimationElement.h>
#include <NeoController/Layer.h>

#include <string>

namespace Xasin {
namespace Animation {

class Eyes: public AnimationElement {
private:
	void draw_color(int x, int y, const Color &color);
	void draw_buffer(const uint8_t *start, int start_x, int start_y, const Color &color);

	void mask_buffer(uint8_t *buffer, const uint8_t *mask, int mask_offset, bool mask_dir);

	void draw_eye();

public:
	Layer &target_layer;
	const int offset;

	Color outer_color;
	Color inner_color;

	float top_rest;
	float bottom_rest;
	float eye_close_factor;

	float iris_x;
	float iris_y;

	std::string disp_string;
	float scroll_speed;
	float scroll_pos;

	Eyes(AnimationServer &server, animation_id_t ID, Layer &target_layer);

	float *get_flt(animation_value_id_t val);
	Color *get_color(uint8_t val);

	void tick(float delta_t);
};

}
} /* namespace Xasin */

#endif /* STM32F4_ANIMATION_EYES_H_ */
