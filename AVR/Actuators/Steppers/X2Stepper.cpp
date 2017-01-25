/*
 * X2Stepper.cpp
 *
 *  Created on: 24.10.2016
 *      Author: xasin
 */

#include "X2Stepper.h"

namespace X2 {
	
Stepper::Stepper(volatile uint8_t *P, uint8_t pins, uint8_t pind,
		uint16_t CPISR, float stepsPerMM, float stepsPerDeg) : PrimitiveStepper(P, pins, pind, CPISR),
				stepsPerMM(stepsPerMM), stepsPerDeg(stepsPerDeg) {
}

void Stepper::ISRStepBy(float x, float r) {
	this->PrimitiveStepper::moveBetweenCalls(x * this->stepsPerMM + r * this->stepsPerDeg);
}
} /* namespace X2 */
