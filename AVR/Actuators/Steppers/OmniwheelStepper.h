/*
 * OmniwheelStepper.h
 *
 *  Created on: 10.11.2015
 *      Author: xasin
 */

#ifndef CODE_OmniwheelStepper_H_
#define CODE_OmniwheelStepper_H_

#include "PrimitiveStepper.h"
#include "../../Movement/X3/X3-Motor.h"

#include <avr/io.h>
#include <util/atomic.h>
#include <math.h>

#define DEG_TO_RAD 0.017453293

namespace X3 {
	class OmniwheelStepper : public PrimitiveStepper, public Motor {

	private:
		//Conversion factors for 1mm of X/Y movement or 1 degree rotation into the amount of steps the robot has to do.
		float xFact, yFact, rFact;
		float const yRCompensation;

	public:
		//Contstructor for a new Translative Stepper.
		//*PORT is a pointer to the port of the motor, pins the start of the pin (Step, then Dir)
		//ISRPerCal is the difference in frequencies of the stepper motor "update" function and the recalculation of X and Y movements.
		//"microstepping" is the microstepping value of the stepper motor
		//Radius is the wheel radius for the motor, rotation the Z-Axis rotation facing towards X-Movement, and distance the distance of the wheel to the center of the robot.
		//OmniwheelStepper(volatile uint8_t *PORT, uint8_t pins, uint16_t ISRPerCal, uint8_t microstepping, float radius, float rotation, float distance, float yRComp);
		OmniwheelStepper(volatile uint8_t *PORT, uint8_t pins, uint8_t pind, uint16_t ISRPerCal, uint8_t microstepping, float radius, float rotation, float distance);

		//Move the motor by x and y mm, and rotate it by r degrees
		void stepBy(float x, float y, float r);
		//Move the motor by x and y mm
		void stepBy(float x, float y);
		//Rotate the robot by r degrees
		void stepBy(float r);
	};
}

#endif /* CODE_OmniwheelStepper_H_ */
