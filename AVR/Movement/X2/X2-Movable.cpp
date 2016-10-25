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
Movable::Movable(uint16_t updateFrequency) {
	this->updateFrequency = updateFrequency;
};

// Set the rotation speed
void Movable::setRotationSpeed(float speed) {
	this->rSpeed = clamp(speed, SANE_RSPEED_MAX)/updateFrequency;
}
// Set the movement speed
void Movable::setSpeed(float speed) {
	this->mSpeed = clamp(speed, SANE_MSPEED_MAX)/updateFrequency;
}

void Movable::rotateBy(float angle) {
	this->mode = relative;
	this->rAngle += angle;
}

void Movable::moveBy(float distance) {
	this->mode = relative;
	this->mDistance += distance;
}

void Movable::continuousMode() {
	this->mode = continuous;
	this->mDistance = 0;
	this->rAngle = 0;
}

bool Movable::atPosition() {
	return fabs(this->mDistance) < 0.1;
}
bool Movable::atRotation() {
	return fabs(this->rAngle) < 0.1;
}
bool Movable::isReady() {
	return (this->atPosition() && this->atRotation());
}

void Movable::flush() {
	while(!this->isReady()) {
		_delay_ms(1);
	}
}

void Movable::update() {
	float xThisCal;
	float rThisCal;

	switch(mode) {

		case relative:
		xThisCal = clamp(mDistance, fabs(mSpeed));
		rThisCal = clamp(rAngle, fabs(rSpeed));

		mDistance -= xThisCal;
		rAngle -= rThisCal;

		Actuator::ISRStepAllBy(xThisCal, rThisCal);
		break;

		case continuous:
		Actuator::ISRStepAllBy(mSpeed, rSpeed);
		break;
	}
}

}
