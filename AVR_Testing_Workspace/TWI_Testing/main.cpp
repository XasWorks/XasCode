/*
 * main.cpp
 *
 *  Created on: 16.03.2017
 *      Author: xasin
 */

#include "AVR/Communication/NEW_TWI/TWI.h"

#include <avr/io.h>
#include <util/delay.h>

uint16_t PINEXPAND = 0;

ISR(TWI_vect) {
	TWI::updateTWI();
}

void updatePINEX() {
	TWI::sendPacketTo(0b01000000, 0x12, &PINEXPAND, 1);
}

int main() {
	TWI::init();

	DDRB |= 1<<5;

	PINEXPAND = 0;
	TWI::sendPacketTo(0b01000000, 0x00, &PINEXPAND, 2);

	while(1) {
		_delay_ms(1000);

		updatePINEX();
		PINEXPAND = 1;

		updatePINEX();
		PINEXPAND = 0;
	}

	return 0;
}
