/*
 * X2Stepper.cpp
 *
 *  Created on: 24.10.2016
 *      Author: xasin
 */

#include "X2Stepper.h"

namespace X2 {

// Initialization of head stepper node.
Stepper *Stepper::headStepper = 0;

Stepper::Stepper(volatile uint8_t *P, uint8_t pins, uint8_t pind,
		uint16_t CPISR, float stepsPerMM, float stepsPerDeg) : PrimitiveStepper(P, pins, pind, CPISR) {

	this->stepsPerMM = stepsPerMM;
	this->stepsPerDeg = stepsPerDeg;

	// Queue-Initialisation
	this->nextStepper = Stepper::headStepper;
	Stepper::headStepper = this;
}

void Stepper::ISRStepBy(float x, float r) {
	this->PrimitiveStepper::moveBetweenCalls(x * this->stepsPerMM + r * this->stepsPerDeg);
}

void Stepper::ISRStepAllBy(float x, float r) {
	X2::Stepper *node = X2::Stepper::headStepper;
	while(node != 0) {
		node->ISRStepBy(x, r);
		node = node->nextStepper;
	}
}
} /* namespace X2 */
