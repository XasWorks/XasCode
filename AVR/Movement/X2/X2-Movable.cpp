/*
 * X2-Movable.cpp
 *
 *  Created on: Oct 5, 2016
 *      Author: xasin
 */

#include "X2-Movable.h"

// If in is below max, return in
// Otherwise however, return max with the sign of in, so that no value greater than max is returned
float clamp(float in, float max) {
	return (fabs(in) > fabs(max)) ?
		copysign(max, in) :
		in;
}

namespace X2 {

// Empty constructor
Movable::Movable() {};

// Set the rotation speed
void Movable::setRotationSpeed(float speed) {
	this->rSpeed = clamp(speed, SANE_RSPEED_MAX);
}
// Set the movement speed
void Movable::setSpeed(float speed) {
	this->mSpeed = clamp(speed, SANE_MSPEED_MAX);
}

// Rotate to a specific angle
void Movable::rotateTo(float angle) {

}

}
