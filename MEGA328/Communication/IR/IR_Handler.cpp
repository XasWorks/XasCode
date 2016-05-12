/*
 * IR_Handler.cpp
 *
 *  Created on: 12.05.2016
 *      Author: xasin
 */

#include "IR_Handler.h"

namespace IR {
	IR_LED *led;

	// Function pointer to be executed upon message received
	void (*on_received)();

	// Message variables for: Received message and message length
	uint8_t mLength = 0;
	uint32_t message = 0;

	// Variables for the reading system
	uint8_t readPosition = 0;
	uint8_t readStage = 0;
	uint8_t readChecksum = 0;

	// Variables for the sending system
	uint32_t outputMessage = 0;
	uint8_t outputLength = 0;
	uint8_t outputPosition = 0;
	uint8_t outputStage = 0;
	uint8_t outputChecksum = 0;

	volatile uint8_t *PORT;
	uint8_t pin;

	void init(volatile uint8_t *PORT, uint8_t pin, IR_LED *led) {
		IR::led = led;

		IR::PORT = PORT;
		IR::pin = pin;

		*(PORT - 1) |= (1<< pin);
	}

	void send() {
		switch(outputStage) {
		case IR_STAGE_IDLE:
			break;

		case IR_STAGE_START:
			led->on();
			if(++outputPosition == IR_START_LEN) {
				if(outputLength == IR_LENGTH_8) {
					led->off();

					outputStage = IR_STAGE_DATA;
					outputPosition = 0;
				}
				else {

					outputStage = IR_STAGE_LENGTH;
					outputPosition = outputLength;	// Set it to output length for slightly more processor-friendly system
				}
			}
		break;

		case IR_STAGE_LENGTH:
			if(--outputPosition == 0) {
				led->off();

				outputStage = IR_STAGE_DATA;
			}
		break;

		case IR_STAGE_DATA:
			led->on();
			if(outputPosition == (0b100 << outputLength)) { 	//Check if the output position equals the amount of data to be sent.
				if((outputChecksum & 0b1) == 0)
					led->off();

				outputPosition = 0;
				outputStage = IR_STAGE_CHECKSUM;
			}
			else {

				if((outputMessage & (1<< outputPosition++)) == 0)
					led->off();
				else
					outputChecksum++;
			}
		break;


		case IR_STAGE_CHECKSUM:
			led->off();
			if(++outputPosition == outputLength) {
				outputChecksum = 0;
				outputPosition = 0;
				outputStage = IR_STAGE_IDLE;
			}
			else if((outputChecksum & (1 << outputPosition)) == 0)
				led->on();
		}
	}

	void update() {
		send();
	}
}
