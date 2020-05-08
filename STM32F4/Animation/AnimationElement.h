/*
 * AnimationElement.h
 *
 *  Created on: 15 Mar 2020
 *      Author: xasin
 */

#ifndef STM32F4_ANIMATION_ANIMATIONELEMENT_H_
#define STM32F4_ANIMATION_ANIMATIONELEMENT_H_

#include <Animation/AnimationServer.h>
#include <config.h>

#include <vector>
#include <map>

#ifndef ANIM_USE_CUSTOM_COORDS
struct led_coord_t {
	float x;
	float y;
};
#endif

namespace Xasin {

class AnimationElement {
protected:
	friend AnimationServer;

	const int draw_z;

public:
	static led_coord_t const * led_coordinates;
	static float calc_coords(int led, led_coord_t coords);
	static float calc_coords(led_coord_t led, led_coord_t coords);

	float delete_after;

	AnimationServer &server;
	const animation_id_t ID;

	AnimationElement(AnimationServer &server, animation_id_t ID, int z = 0);
	virtual ~AnimationElement();

	virtual animation_flt_val_t * get_flt(uint8_t val_num);

	void set_flt(uint8_t val, const char *description);

	virtual void tick(float delta_t);
	virtual void relink();
};

} /* namespace Xasin */

#endif /* STM32F4_ANIMATION_ANIMATIONELEMENT_H_ */
