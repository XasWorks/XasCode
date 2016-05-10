/*
 * 3X-Locomotor.h
 *
 *  Created on: Nov 11, 2015
 *      Author: xasin
 */

#ifndef CODE_3X_Locomotor_H_
#define CODE_3X_Locomotor_H_

#include <math.h>

#include "../X3/X3-Motor.h"
#include "../X3/X3-Movable.h"

namespace X3 {
	class Locomotor : public Movable {
	private:

		//Distance that the motors have to cover per Calculation
		float xFact = 0, yFact = 0, rPerISR = 0;

		float speedTarget = 0, acceleration = 30;

	#define ACCEL_DEBOUNCE_CYCLES 3
		uint8_t accelDebounce = ACCEL_DEBOUNCE_CYCLES;

		//Speed at which the robot will recalculate
		uint16_t ISRFreq;

		//Recalculate movement per ISR factors of X and Y axis
		void recalculateXYFact();

		//Code to calculate the robots accelleration value
		void accelerate();

	public:
		//Constructor for the 3X-Locomotor. It requires three pointers to instances of the Translative Stepper class with appropriately set values, as well as the ISR Speed the Update function will be executed at.
		Locomotor(uint16_t ISRSpeed);

		//Set the rotation speed of the robot in degrees / second
		void setRotationSpeed(float newSpeed);
		//Accelerate the robot towards a wanted speed.
		void setSpeed(float speed);

		//Set the acceleration of the robot in mm / second²
		void setAcceleration(float acceleration);

		//Move the robot to the specified X and Y coordinates, in mm.
		void moveTo(float x, float y);
		//Move the robot by the specified amount of mm along X and Y axis
		void moveBy(float x, float y);

		//Move the robot forwards by "dist" mm
		void moveTowards(float dist);
		//Move the robot towards the specified direction in degrees by "dist" mm.
		void moveTowards(float dist, float direction);

		//Wait for all movements to finish.
		void flush();

		//Update the robot calculation, must be executed within a regular and slow ISR (~75Hz)
		void update();
	};
}

#endif /* CODE_3X-Locomotor_H_ */
