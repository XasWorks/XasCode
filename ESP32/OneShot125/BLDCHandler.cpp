/*
 * BLDCHandler.cpp
 *
 *  Created on: 9 Aug 2019
 *      Author: xasin
 */

#include "xasin/BLDCHandler.h"

namespace Xasin {
namespace Drone {

BLDCHandler::BLDCHandler(uint8_t numCh, uint8_t gpio_start)
	: num_channels(numCh), gpio_start(gpio_start) {
}

BLDCHandler::~BLDCHandler() {
}

void BLDCHandler::set_motor_power(uint8_t id, float val) {
}

float BLDCHandler::get_motor_power(uint8_t id) {
	return 0;
}

}
} /* namespace Xasin */
