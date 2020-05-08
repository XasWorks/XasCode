/*
 * Box.h
 *
 *  Created on: 23 Mar 2020
 *      Author: xasin
 */

#ifndef STM32F4_ANIMATION_BOX_H_
#define STM32F4_ANIMATION_BOX_H_

#include <Animation/AnimationElement.h>
#include <NeoController/Layer.h>

namespace Xasin {
namespace Animation {

class Box: public AnimationElement {
public:
	Layer &target;

	led_coord_t x_coord;
	led_coord_t y_coord;
	led_coord_t center;

	float rotation;
	float up, down, left, right;

	Color draw_color;

	Box(AnimationServer &server, animation_id_t ID, Layer &tgt_layer, int z = 0);

	float * get_flt(animation_value_id_t val);
	Color * get_color(uint8_t val);

	void tick(float delta_t);
};

} /* namespace Animation */
} /* namespace Xasin */

#endif /* STM32F4_ANIMATION_BOX_H_ */
