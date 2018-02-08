#ifndef _PRIMITIVE_STEPPER_H
#define _PRIMITIVE_STEPPER_H

#include <avr/io.h>
#include <util/atomic.h>
#include <math.h>


class PrimitiveStepper {
private:
	//Port of the stepper motor
	volatile uint8_t * const PORT;
	//Pins of the stepper motor. Stepping is issued at pins, direction at pind
	uint8_t const pind, pins;

	//Speed of the ISR. Required to set the stepping speed correctly.
	uint16_t const updateFrequency;

	//Total steps the motor has made. Useful for absolute positioning.
	volatile int32_t currentSteps = 0;
	//Steps the motor still has to make.
	float stepsToGo = 0;

	//Steps per ISR Call.
	float stepSpeed = 1;
	//Step buffer of the system. Not every ISR call will induce a Step, thusly, the rest has to be buffered.
	float stepBuffer = 0;

	//Step the motor ONCE into the specified direction (0 == backwards, else forwards)
	void step(uint8_t dir);

protected:
	//Makes the motor move "steps", over a total time of "updateSpeed" ISR calls. Especially useful for more advanced systems (Omniwheel movement).
	void moveBetweenCalls(float steps);

public:

	//Main constructor of the Stepper motor. P is the port, p the first of the two pins required for the motor.
	//updateFrequency specifies the ISR speed that the motor runs on OR! the amount of ISR calls between lower-speed calculation calls.
	PrimitiveStepper(volatile uint8_t *P, uint8_t pins, uint8_t pind,
			volatile uint16_t updateFrequency);

	//Deprecated constructor, taking only one pin, instead of step and direction pins.
	PrimitiveStepper(volatile uint8_t *P, uint8_t pin,
			volatile uint16_t updateFrequency);

	//ISR Function to update the stepper motor (Stepping it when required)
	virtual void update();

	//Return the speed of the motor in steps/second
	virtual float getSpeed();
	//Return the current position of the motor as given in currentSteps
	virtual float getPosition();
	//Return the updating speed of the stepper motor - can be useful for some calculations
	virtual uint16_t getFrequency();

	//Set the speed in steps per second.
	virtual void setSpeed(float stepsPerSecond);

	// Move the stepper motor by a certain amount of steps
	virtual void move(float steps);
	virtual void disregardAndMove(float steps);

	//Wait for every motor move of this motor to finish.
	virtual void flush();
	//Reset the motor and abort all movements.
	virtual void reset();
};

#endif
