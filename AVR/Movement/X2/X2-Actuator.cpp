/*
 * X2-Actuator.cpp
 *
 *  Created on: 24.10.2016
 *      Author: xasin
 */

#include "X2-Actuator.h"

namespace X2 {

// Initialization of the head stepper
Actuator *Actuator::headActuator = 0;

Actuator::Actuator() {
	// Initialization of the Actuator Queue
	this->nextActuator = Actuator::headActuator;
	Actuator::headActuator = this;
}

void Actuator::ISRStepBy(float x, float r) {}

void Actuator::ISRStepAllBy(float x, float r) {
	Actuator *node = Actuator::headActuator;
	while(node != 0) {
		node->ISRStepBy(x, r);
		node = node->nextActuator;
	}
}

} /* namespace X2 */
