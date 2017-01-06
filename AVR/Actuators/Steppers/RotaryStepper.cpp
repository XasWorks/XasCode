/*
 *
 *  * RotaryStepper.cpp
 *
 *  Created on: Oct 7, 2015
 *      Author: xasin
 */

#include "RotaryStepper.h"

//Constructor of the stepper motor.
//Takes in PORT Pointer, PIN, ISR Frequency and Steps/Rotation
RotaryStepper::RotaryStepper(volatile uint8_t *P, uint8_t pinMotor,
		uint16_t upSp, int16_t stepsPerRotation)
		: PrimitiveStepper(P, pinMotor, upSp), stepsPerRotation(stepsPerRotation) {
}

//Set the speed of the motor to the given amount of degrees/sec
void RotaryStepper::setSpeed(float degreePerSec) {
	//Calculate the required amount of steps per second.
	this->PrimitiveStepper::setSpeed((degreePerSec * stepsPerRotation) / 360);
}

float RotaryStepper::getPosition() {
	return ((float)this->PrimitiveStepper::getPosition() / stepsPerRotation) / 360;
}

//Rotate the stepper to an absolute position in degrees.
void RotaryStepper::rotateTo(float target) {
	this->PrimitiveStepper::disregardAndMove((target * stepsPerRotation)/360 - this->PrimitiveStepper::getPosition());
	//Get the required steps it has to perform,
	//Then set the difference of current and wanted position into the toGo variable.
}

//Move the stepper motor by a relative amount.
void RotaryStepper::move(float amount) {
	this->PrimitiveStepper::move((amount * stepsPerRotation)/360);
	//Calculate the required amount of steps and add to the toGo variable.
}
