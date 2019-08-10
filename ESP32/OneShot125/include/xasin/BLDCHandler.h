/*
 * BLDCHandler.h
 *
 *  Created on: 9 Aug 2019
 *      Author: xasin
 */

#ifndef ESP32_ONESHOT125_BLDCHANDLER_H_
#define ESP32_ONESHOT125_BLDCHANDLER_H_

#include <stdint.h>

namespace Xasin {
namespace Drone {

class BLDCHandler {
public:
	const uint8_t num_channels;
	const uint8_t gpio_start;

	BLDCHandler(uint8_t channel_count, uint8_t gpio_start);
	virtual ~BLDCHandler();

	virtual void  set_motor_power(uint8_t id, float val);
	virtual float get_motor_power(uint8_t id);
};

}
} /* namespace Xasin */

#endif /* ESP32_ONESHOT125_BLDCHANDLER_H_ */
