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

void RGBStatus::update() {
	if(++this->sequencePosition == 12)
		sequencePosition = 0;

	*PORT &= ~(1<< this->pinR | 1<< this->pinG | 1<< this->pinB);
	if(RMode & (1 << sequencePosition))
		*PORT |= (1<< this->pinR);
	if(GMode & (1 << sequencePosition))
		*PORT |= (1<< this->pinG);
	if(BMode & (1 << sequencePosition))
		*PORT |= (1<< this->pinB);
}

} /* namespace Communication */
