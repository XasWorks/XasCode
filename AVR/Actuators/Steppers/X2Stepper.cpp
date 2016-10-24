/*
 * X2Stepper.cpp
 *
 *  Created on: 24.10.2016
 *      Author: xasin
 */

#include "X2Stepper.h"

namespace X2 {

X2Stepper::X2Stepper(volatile uint8_t *P, uint8_t pins, uint8_t pind,
		uint16_t CPISR, float stepsPerMM, float stepsPerDeg) : PrimitiveStepper(P, pins, pind, CPISR) {

	this->stepsPerMM = stepsPerMM;
	this->stepsPerDeg = stepsPerDeg;
}

void X2Stepper::ISRStepBy(float x, float r) {
	this->PrimitiveStepper::moveBetweenCalls(x * this->stepsPerMM + r * this->stepsPerDeg);
}

} /* namespace X2 */
