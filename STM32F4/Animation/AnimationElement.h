/*
 * AnimationElement.h
 *
 *  Created on: 15 Mar 2020
 *      Author: xasin
 */

#ifndef STM32F4_ANIMATION_ANIMATIONELEMENT_H_
#define STM32F4_ANIMATION_ANIMATIONELEMENT_H_

#include <Animation/AnimationServer.h>

namespace Xasin {

class AnimationElement {
public:
	AnimationServer &server;
	const animation_id_t ID;

	AnimationElement(AnimationServer &server, animation_id_t ID);
	virtual ~AnimationElement();

	virtual animation_flt_val_t * get_flt(uint8_t val_num);

	void set_flt(uint8_t val, const char *description);

	virtual void tick(float delta_t);
	virtual void relink();
};

} /* namespace Xasin */

#endif /* STM32F4_ANIMATION_ANIMATIONELEMENT_H_ */
