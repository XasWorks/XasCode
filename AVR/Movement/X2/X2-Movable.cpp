/*
 * X2-Movable.cpp
 *
 *  Created on: Oct 5, 2016
 *      Author: xasin
 */

#include "X2-Movable.h"
#include <util/atomic.h>

// If in is below max, return in
// Otherwise however, return max with the sign of in, so that no value greater than max is returned
float clamp(float in, float max) {
	return (fabs(in) > fabs(max)) ?
		copysign(max, in) :
		in;
}

namespace X2 {

// Empty constructor
Movable::Movable(uint16_t updateFrequency) : updateFrequency(updateFrequency) {
}

// Set the rotation speed
void Movable::setRotationSpeed(float speed) {
	float bufSpeed = clamp(speed, SANE_RSPEED_MAX)/(float)updateFrequency;
	ATOMIC_BLOCK(ATOMIC_FORCEON) {
		this->rSpeed = bufSpeed;
	}
}
// Set the movement speed
void Movable::setSpeed(float speed) {
	float bufSpeed = clamp(speed, SANE_MSPEED_MAX)/(float)updateFrequency;
	ATOMIC_BLOCK(ATOMIC_FORCEON) {
		this->mSpeed = bufSpeed;
	}
}
void Movable::setSpeeds(float mSpeed, float rSpeed) {
	this->setRotationSpeed(rSpeed);
	this->setSpeed(mSpeed);
}

void Movable::rotateBy(float angle) {
	ATOMIC_BLOCK(ATOMIC_FORCEON) {
		this->rAngle = angle;
	}
	this->mode = relative;
}
void Movable::rotateF(float angle) {
	this->rotateBy(angle);
	this->flush();
}

void Movable::moveBy(float distance) {
	ATOMIC_BLOCK(ATOMIC_FORCEON) {
		this->mDistance = distance;
	}
	this->mode = relative;
}
void Movable::moveF(float distance) {
	this->moveBy(distance);
	this->flush();
}

void Movable::continuousMode() {
	this->mode = continuous;
		// No atomic operation needed as these won't be accessed later on!
	this->mDistance = 0;
	this->rAngle = 0;
}
void Movable::continuousMode(float mSpeed, float rSpeed) {
	this->setSpeeds(mSpeed, rSpeed);
	this->continuousMode();
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
void Movable::cancel() {
	ATOMIC_BLOCK(ATOMIC_FORCEON) {
		this->mDistance = 0;
		this->rAngle = 0;
	}
	this->mode = relative;
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
		movedDistance += xThisCal;
		movedRotation += rThisCal;
		break;

		case continuous:
		Actuator::ISRStepAllBy(mSpeed, rSpeed);
		movedDistance += mSpeed;
		movedRotation += rSpeed;
		break;
	}
}

}
