/*
 * ShiftReg.cpp
 *
 *  Created on: 15.12.2016
 *      Author: xasin
 */

#include "ShiftReg.h"

namespace Interfacing {

void ShiftReg::writeByte(uint8_t data) {
	uint8_t i=7;
	while(true) {
		if((data & (1 << i)) != 0)
			SET_DATA;
		SEND_SRCK;
		UNSET_DATA;
		if(i-- == 0)
			break;
	}
}

void ShiftReg::write8(uint8_t data) {
	this->writeByte(data);
	SEND_RCK;
}
void ShiftReg::write16(uint16_t data) {
	this->writeByte(data >> 8);
	this->writeByte(data);
	SEND_RCK;
}
void ShiftReg::write32(uint32_t data) {
	this->writeByte(data >> 24);
	this->writeByte(data >> 16);
	this->writeByte(data >> 8);
	SEND_RCK;
}

void ShiftReg::write(void * const data, uint8_t length) {
	while(length !=0) {
		this->writeByte(*(uint8_t *)(data + --length));
	}

	SEND_RCK;
}

ShiftReg::ShiftReg(volatile uint8_t * const PORT, const uint8_t pin) :
	PORT(PORT), pin(pin) {

	// Initialise shift register outputs
	*(this->PORT -1) |= (0b111<< this->pin);
}

} /* namespace Interfacing */
