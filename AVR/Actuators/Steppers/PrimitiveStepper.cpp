#include <util/delay.h>

#include "PrimitiveStepper.h"

//Step the motor once into specified direction.
void PrimitiveStepper::step(uint8_t dir) {
	*PORT &= ~(1 << this->pins | 1 << this->pind);	// Deactivate both pins
	if (dir == 0) {					//Rotate it backwards
		*PORT |= (1 << this->pind);	//Set the direction pin, then activate the stepping pin
		*PORT |= (1 << this->pins);
		stepsToGo++;				//One step backwards was done; increase the amount of steps to do and decrease the total amount
		currentSteps--;
	} else {						//Rotate it forwards
		*PORT |= (1 << this->pins);	//Activate the step pin
		stepsToGo--;				//Decrease the steps to do and increase total amount of steps done.
		currentSteps++;
	}
}

void PrimitiveStepper::moveBetweenCalls(float steps) {
	this->stepsToGo += steps;
	this->stepBuffer = 0;

	this->stepSpeed = fabs(stepsToGo) / (float)this->updateFrequency;
}

PrimitiveStepper::PrimitiveStepper(volatile uint8_t *P, uint8_t pins, uint8_t pind,
		uint16_t upSpeed)
		: PORT(P),  pind(pind), pins(pins), updateFrequency(upSpeed) {

	*(P - 1) |= (1 << pins | 1 << pind);
}

//Deprecated constructor for the motor stepper. Takes in port pointer, pin and ISR frequency.
PrimitiveStepper::PrimitiveStepper(volatile uint8_t *P, uint8_t pin,
		uint16_t upSpeed) : PrimitiveStepper(P, pin, pin + 1, upSpeed) {
}

//ISR Routine for the motor, updates it when required.
void PrimitiveStepper::update() {
	*PORT &= ~(1 << this->pins | 1 << this->pind);	// Deactivate pind and pins for the step routine

	if (fabs(stepsToGo) >= 1) {	//If there are any steps to do at all

		stepBuffer += stepSpeed;	//Increase the "Buffer step" value by the Steps/ISR value

		if (stepBuffer >= 1) {	//Is there a step to be done?
			stepBuffer -= 1;

			if (signbit(stepsToGo))	//Backwards movement
				step(0);
			else 					//Forwards movement
				step(1);
		}
	}
}

float PrimitiveStepper::getSpeed() {
	return (signbit(stepsToGo) ? -1 : 1) * this->stepSpeed;
}
float PrimitiveStepper::getPosition() {
	return this->currentSteps;
}
uint16_t PrimitiveStepper::getFrequency() {
	return this->updateFrequency;
}

//Set the speed of the motor in steps per second.
void PrimitiveStepper::setSpeed(float stepsPerSec) {
	ATOMIC_BLOCK(ATOMIC_FORCEON) {
		stepSpeed = stepsPerSec / updateFrequency;	//Calculate the required steps per ISR call.
	}
}

//Move the stepper motor by the specified amount of steps.
void PrimitiveStepper::move(float steps) {
	ATOMIC_BLOCK(ATOMIC_FORCEON) {
		stepsToGo += steps;
	}
}
//Move the stepper motor, disregarding any previous movement
void PrimitiveStepper::disregardAndMove(float steps) {
	ATOMIC_BLOCK(ATOMIC_FORCEON) {
		stepsToGo = steps;
	}
}


//Wait for the motor movements to finish.
void PrimitiveStepper::flush() {
	while ((fabs(stepsToGo) >= 1) && (stepSpeed != 0)) {
		_delay_ms(1);
	}
}
//Reset all the values back to 0, except PORT and PIN configurations.
void PrimitiveStepper::reset() {
	stepsToGo = 0;
	currentSteps = 0;
	stepBuffer = 0;
	stepSpeed = 0;
}
