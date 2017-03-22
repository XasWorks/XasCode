/*
 * main.cpp
 *
 *  Created on: 16.03.2017
 *      Author: xasin
 */

#include "AVR/Communication/NEW_TWI/TWI.h"

#include <avr/io.h>
#include <util/delay.h>

#include "AVR/Communication/NEW_TWI/TWIJobs/MCPIOEXP.h"

TWI::MCP_IOEXP mIO = TWI::MCP_IOEXP(0b01000000);

ISR(TWI_vect) {
	TWI::updateTWI();
}

void updatePINEX() {
	mIO.update();
	TWI::checkMasterJobs();
}

int main() {
	TWI::init();

	DDRB |= 1<<5;

	DDRD |= 0b11111100;

	updatePINEX();
	_delay_ms(3);
	updatePINEX();

	mIO.DDRS &= ~1;

	while(1) {
		_delay_ms(1);
		updatePINEX();

	   if((mIO.PINS & 0b10) != 0)
			mIO.PORTS = 0b11;
	   else
		   mIO.PORTS = 0b10;
	}

	return 0;
}
