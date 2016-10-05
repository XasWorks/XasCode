/*
 * X2-Movable.h
 *
 *  Created on: Oct 5, 2016
 *      Author: xasin
 */

#ifndef AVR_MOVEMENT_X2_X2_MOVABLE_H_
#define AVR_MOVEMENT_X2_X2_MOVABLE_H_

#include <util/atomic.h>
#include <math.h>

#define SANE_RSPEED_MAX 180
#define SANE_MSPEED_MAX 200

namespace X2 {

class Movable {
protected:
	// Movement Variables.
	// These are given in standard units (mm and degrees)
		// Variables for rotation (speed, angle and est. position)
		volatile float rAngle = 0, rPosition = 0;
		float rSpeed = 0;

		// Variables for linear movement (speed and distance)
		volatile float mDistance = 0;
		float mSpeed = 0;

public:
	// Standard constructor. Needs no arguments
	Movable();

	// Set rotation speed of the robot in deg/second
	virtual void setRotationSpeed(float speed);
	// Set movement speed in mm / second
	virtual void setSpeed(float speed);

	// Rotate to a specific heading
	virtual void rotateTo(float angle);
	// Rotate by a specific amount of degrees
	virtual void rotateBy(float angle);

	// Move the robot forwards by the specified mm
	virtual void moveBy(float distance);

	// Set the robot into constant rotation/movement mode
	// This is done by setting the "leftover movement" variables to infinity,
	// This also makes the system automatically halt once it receives a "normal" movement command
	virtual void continuousRotation(float speed);
	virtual void continuousMove(float speed);

	// Waits for all moves to complete (will not wait for continuous movements to finish!)
	virtual void flush();

	// Returns true if the robot has finished all non-continuous moves
	virtual bool isReady();
	// Returns true if the robot has finished linear movement
	virtual bool atPosition();
	// Returns true if the robot has finished rotation movement
	virtual bool atRotation();


	// Return the current rotation of the robot
	virtual float getRotation();
};

}


#endif /* AVR_MOVEMENT_X2_X2_MOVABLE_H_ */
