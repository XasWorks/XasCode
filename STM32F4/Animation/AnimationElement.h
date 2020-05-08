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
private:
	void copy_op_tick(float delta_t);
	void color_op_tick(float delta_t);

protected:
	friend AnimationServer;

	const int draw_z;

public:
	static led_coord_t const * led_coordinates;
	static float calc_coords(int led, led_coord_t coords);
	static float calc_coords(led_coord_t led, led_coord_t coords);

	float delete_after;
	std::map<animation_value_id_t, animation_copy_op> copy_ops;
	std::map<uint8_t, animation_color_op> color_ops;

	AnimationServer &server;
	const animation_id_t ID;

	AnimationElement(AnimationServer &server, animation_id_t ID, int z = 0);
	virtual ~AnimationElement();

	void delete_copy_to(animation_value_id_t val_num);
	animation_copy_op & get_copy_to(animation_value_id_t val_num);

	virtual float * get_flt(animation_value_id_t val_num);
	void set_flt(animation_value_id_t val_num, const char *command);
	void set_flt(animation_value_id_t val_num, float new_val);
	void set_flt(animation_value_id_t val_num, animation_global_id_t source);
	void set_flt_op(animation_value_id_t val_num, animation_copy_op op);

	virtual Color * get_color(uint8_t val_num);
	void set_color(uint8_t value, const char *command);
	void set_color(uint8_t value, Color n_color);
	void set_color_op(uint8_t val_num, animation_color_op op);

	virtual void tick(float delta_t);

	void copy_tick(float delta_t);
	void relink();
};

} /* namespace Xasin */

#endif /* STM32F4_ANIMATION_ANIMATIONELEMENT_H_ */
