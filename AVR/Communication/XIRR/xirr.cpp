/*
 * xirr.cpp
 *
 *  Created on: 31 May 2019
 *      Author: xasin
 */

#include "xirr.h"
#include "../../TIMER/Timer0.h"
#include "../../TIMER/Timer1.h"

#include <string.h>

namespace Communication {
namespace XIRR {

namespace RX {
enum rx_state_t {
	IDLE,
	START,
	DATA
} state = IDLE;
uint8_t buffer[32] = {0};
uint8_t bitCounter = 0;
uint8_t byteCounter = 0;

void (*process_data)(const uint8_t *data, uint8_t length, uint8_t key) = nullptr;

bool read_pin() {
	return (PINB & 1) == 0;
}

void pin_capture() {
	if(state == IDLE)
		state = START;

	// Adjust the frame timing a little forwards to re-synch with the signal
	uint16_t tempOCR1B = ICR1 + OCR1A/2;
	if(tempOCR1B >= OCR1A)
		tempOCR1B -= OCR1A;
	OCR1B = tempOCR1B;
}

void process_raw_data() {
	for(uint8_t i=0; i<(byteCounter-1); i++) {
		buffer[byteCounter-1] -= buffer[i];
	}

	if(buffer[byteCounter-1] != 0)
		return;

	if(process_data != nullptr)
		process_data(buffer+1, byteCounter-2, buffer[0]);
}

void update() {
	switch(state) {
	case IDLE:
	case START:
		if(read_pin() ^ (bitCounter == 7))
			bitCounter++;
		else {
			bitCounter = 0;
			state = IDLE;
		}

		if(bitCounter == 8) {
			bitCounter = 0;
			byteCounter = 0;
			state = DATA;
		}
	break;

	case DATA:
		if((bitCounter < 8)) {
			if((byteCounter < sizeof(buffer)) && read_pin())
				buffer[byteCounter] |= (1<<bitCounter);

			bitCounter++;
		}
		else {
			bitCounter = 0;

			byteCounter++;
			if(!read_pin()) {
				state = IDLE;

				process_raw_data();

				memset(buffer, 0, 32);
				byteCounter = 0;
			}
		}
	break;

	}
}

}

namespace TX {
enum tx_state_t {
	OFF,
	START,
	DATA,
};
volatile tx_state_t state = OFF;
volatile uint8_t bitCounter = 0;
volatile uint8_t byteCounter = 0;

uint8_t buffer[32] = {0};
uint8_t byteLength = 0;

void set_led(bool mode) {
	if(mode)
		DDRD |= (1<< PD6);
	else
		DDRD &= ~(1<< PD6);
}

void update() {
	switch(state) {
	case OFF:
		set_led(false);
		bitCounter = 0;
	break;

	case START:
		bitCounter++;

		set_led(bitCounter != 8);
		if(bitCounter == 8) {
			bitCounter = 0;
			byteCounter = 0;
			state = DATA;
		}
	break;

	case DATA:
		if(bitCounter == 8) {
			bitCounter = 0;
			byteCounter++;

			if(byteCounter >= byteLength) {
				state = OFF;
				byteCounter = 0;

				set_led(false);
			}
			else
				set_led(true);

			break;
		}

		set_led((buffer[byteCounter] >> bitCounter) & 1);

		bitCounter++;
	break;
	}
}

void send_raw(const uint8_t *data, uint8_t length) {
	if(length > sizeof(buffer))
		return;
	memcpy(buffer, data, length);

	byteLength = length;
	bitCounter = 0;
	byteCounter = 0;

	state = START;
}

void send(const void *data, uint8_t length, uint8_t key) {
	if((length+2) > sizeof(buffer))
		return;

	buffer[0] = key;

	memcpy(buffer+1, data, length);

	buffer[length+1] = 0;
	for(uint8_t i=0; i<=length; i++) {
		buffer[length+1] += buffer[i];
	}

	byteLength = length+2;
	bitCounter = 0;
	byteCounter = 0;

	state = START;
}

}

void init() {
	Timer0::set_mode(TIMER0_MODE_40KHZ);
	Timer0::set_OCA0_mode(TIMER0_OCA0_TOGGLE);

	Timer1::enable_CTC(2000);
	TINTMASK |= (1<< OCIE1B);

	// Initialize the Input Capture event - Noise Canceler on, Falling Edge. CTC with OCR1A as TOP, no prescaler.
	TCCR1B 	|= (1<< ICNC1 | 1<< WGM12 | 1<< CS10);
	TIMSK1 	|= (1<< ICIE1 | 1<< OCIE1A);

	PORTB |= 1;
}

bool is_idle() {
	if(TX::state != TX::OFF)
		return false;
	if(RX::state != RX::IDLE)
		return false;

	return true;
}

}
}
