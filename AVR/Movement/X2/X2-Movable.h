/*
 * X2-Movable.h
 *
 *  Created on: Oct 5, 2016
 *      Author: xasin
 */

#ifndef AVR_MOVEMENT_X2_X2_MOVABLE_H_
#define AVR_MOVEMENT_X2_X2_MOVABLE_H_

#include <util/atomic.h>
#include <util/delay.h>
#include <math.h>

#include "X2-Actuator.h"

#define SANE_RSPEED_MAX 250
#define SANE_MSPEED_MAX 500

namespace X2 {

enum MovementMode : uint8_t { continuous, relative };

class Movable {
protected:
	// Movement Variables.
	// These are given in standard units for absolute positions,
	// However are given in UNIT/ISR for speed for quicker calculations.
		// Variables for rotation (speed, angle and est. position)
		volatile float rAngle = 0;
		float rSpeed = 0;

		// Variables for linear movement (speed and distance)
		volatile float mDistance = 0;
		float mSpeed = 0;

		uint16_t const updateFrequency;

		MovementMode mode = relative;

public:
	float movedDistance = 0;
	float movedRotation = 0;

	// Standard constructor, taking in only the ISR frequency
	Movable(uint16_t updateFrequency);

	// Set rotation speed of the robot in deg/second
	virtual void setRotationSpeed(float speed);
	// Set movement speed in mm / second
	virtual void setSpeed(float speed);
	// Set both speeds at once
	void setSpeeds(float mSpeed, float rSpeed);

	// Rotate by a specific amount of degrees
	virtual void rotateBy(float angle);
	void rotateF(float angle);
	// Move the robot forwards by the specified mm
	virtual void moveBy(float distance);
	void moveF(float distance);

	// Set the robot into constant rotation/movement mode
	// This is done by setting the "mode" enum to "continuous"
	// This also makes the system automatically halt once it receives a "normal" movement command
	virtual void continuousMode();
	void continuousMode(float mSpeed, float rSpeed);

	// Waits for all moves to complete (will not wait for continuous movements to finish!)
	virtual void flush();

	// Cancels all movements and stops the robot
	virtual void cancel();

	// Returns true if the robot has finished all non-continuous moves
	virtual bool isReady();
	// Returns true if the robot has finished linear movement (or moves continuously)
	virtual bool atPosition();
	// Returns true if the robot has finished rotation movement (or moves continuously)
	virtual bool atRotation();

	virtual void update();
};

}


#endif /* AVR_MOVEMENT_X2_X2_MOVABLE_H_ */
