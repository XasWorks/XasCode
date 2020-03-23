/*
 * WaveElement.h
 *
 *  Created on: 20 Mar 2020
 *      Author: xasin
 */

#ifndef STM32F4_ANIMATION_WAVEELEMENT_H_
#define STM32F4_ANIMATION_WAVEELEMENT_H_

#include <Animation/AnimationElement.h>

#include <NeoController/Layer.h>

namespace Xasin {
namespace Animation {

class WaveElement: public AnimationElement {
public:
	Layer &target;

	float circle_diam;
	float circle_speed;

	led_coord_t x_dir;
	led_coord_t y_dir;

	led_coord_t offset;

	float circle_width;

	Color wave_color;

	WaveElement(AnimationServer &server, animation_id_t ID, Layer &tgt_layer);

	void tick(float delta_t);
};

} /* namespace Animation */
} /* namespace Xasin */

#endif /* STM32F4_ANIMATION_WAVEELEMENT_H_ */
