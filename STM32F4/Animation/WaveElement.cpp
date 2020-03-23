/*
 * WaveElement.cpp
 *
 *  Created on: 20 Mar 2020
 *      Author: xasin
 */

#include <Animation/WaveElement.h>

#include <math.h>

namespace Xasin {
namespace Animation {

WaveElement::WaveElement(AnimationServer &server, animation_id_t ID, Layer &tgt_layer)
	: AnimationElement(server, ID), target(tgt_layer),
	  circle_diam(-5), circle_speed(1),
	  circle_width(5), wave_color() {
}

void WaveElement::tick(float delta_t) {
	circle_diam += delta_t * circle_speed;

	const float offset_x = calc_coords(offset, x_dir);
	const float offset_y = calc_coords(offset, y_dir);
	const float min_diam = powf(fmaxf(circle_diam-circle_width, 0), 2);
	const float max_diam = powf(circle_diam+circle_width, 2);

	const float merge_alpha = (circle_diam < 3.4) ? 1 : (3 / circle_diam);

	for(int i=0; i<target.length(); i++) {
		const float led_x = fabsf(calc_coords(i, x_dir) - offset_x);
		const float led_y = fabsf(calc_coords(i, y_dir) - offset_y);

		if(led_x > max_diam)
			continue;
		if(led_y > max_diam)
			continue;
		if((led_y+led_x) < min_diam)
			continue;

		float led_distance = powf(led_x, 2) + powf(led_y, 2);

		if((led_distance <= max_diam) && (led_distance >= min_diam))
			target[i].merge_overlay(wave_color, merge_alpha);
	}
}

} /* namespace Animation */
} /* namespace Xasin */
