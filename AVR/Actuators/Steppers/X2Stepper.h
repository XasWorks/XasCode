/*
 * X2Stepper.h
 *
 *  Created on: 24.10.2016
 *      Author: xasin
 */

#ifndef AVR_ACTUATORS_STEPPERS_X2STEPPER_H_
#define AVR_ACTUATORS_STEPPERS_X2STEPPER_H_

#include "PrimitiveStepper.h"

namespace X2 {

class Stepper: public PrimitiveStepper {
private:
	// Constants for calculating how many steps the system has to do for forwards/backwards movement, as well as rotation.
	float stepsPerMM, stepsPerDeg;

	static Stepper *headStepper;
	Stepper *nextStepper = 0;

public:
	// Main constructor, initializes values.
	Stepper(volatile uint8_t *P, uint8_t pins, uint8_t pind,
			uint16_t CPISR, float stepsPerMM, float stepsPerDeg);

	void ISRStepBy(float x, float r);
	static void ISRStepAllBy(float x, float r);
};

} /* namespace X2 */

#endif /* AVR_ACTUATORS_STEPPERS_X2STEPPER_H_ */
