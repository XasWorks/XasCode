/*
 * RGBStatus.cpp
 *
 *  Created on: 06.11.2016
 *      Author: xasin
 */

#include "RGBStatus.h"

namespace Communication {

RGBStatus::RGBStatus(volatile uint8_t * const PORT, const uint8_t pinR, const uint8_t pinG, const uint8_t pinB) :
	pinR(pinR), pinG(pinG), pinB(pinB), PORT(PORT) {

	*(PORT - 1) |= (1<< this->pinR | 1<< this->pinG | 1<< this->pinB);

}

void RGBStatus::setModes(uint16_t rMode, uint16_t gMode, uint16_t bMode) {
	this->RMode = rMode & 4095;
	this->GMode = gMode & 4095;
	this->BMode = bMode & 4095;
}

void RGBStatus::signal(uint16_t red, uint16_t green, uint16_t blue, uint8_t length) {
	uint16_t formerR, formerG, formerB;
	formerR = this->RMode;
	formerG = this->GMode;
	formerB = this->BMode;

	length--;

	this->RMode = red;
	this->GMode = green;
	this->BMode = blue;
	this->sequencePosition = 11;
	while(sequencePosition == 11) {
		_delay_ms(10);
	}

	while((sequencePosition < length) && (sequencePosition != 11)) {
		_delay_ms(10);
	}

	this->RMode = formerR;
	this->GMode = formerG;
	this->BMode = formerB;
}

void RGBStatus::update() {
	if(++this->sequencePosition == 12)
		sequencePosition = 0;

	*PORT &= ~(1<< this->pinR | 1<< this->pinG | 1<< this->pinB);
	if(this->RMode & (1 << this->sequencePosition))
		*PORT |= (1<< this->pinR);
	if(this->GMode & (1 << this->sequencePosition))
		*PORT |= (1<< this->pinG);
	if(this->BMode & (1 << this->sequencePosition))
		*PORT |= (1<< this->pinB);
}

} /* namespace Communication */
