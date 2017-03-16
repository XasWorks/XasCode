/*
 * main.cpp
 *
 *  Created on: 16.03.2017
 *      Author: xasin
 */

#include "AVR/Communication/NEW_TWI/TWI.h"


uint16_t PINEXPAND = 0;

ISR(TWI_vect) {
	TWI::updateTWI();
}

void updatePINEX() {
	while(TWI::readSR() != TWI::Status::IDLE) {};
}

int main() {
	TWI::init();

	return 0;
}
