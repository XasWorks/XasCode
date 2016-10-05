/*
 * IR_Handler.h
 *
 *  Created on: 12.05.2016
 *      Author: xasin
 */

#ifndef LIBCODE_COMMUNICATION_IR_IR_HANDLER_H_
#define LIBCODE_COMMUNICATION_IR_IR_HANDLER_H_

#include <avr/io.h>

#include "IR_LED.h"

#define IR_LENGTH_8			1
#define IR_LENGTH_16		2
#define IR_LENGTH_32		3

#define IR_START_LEN		6
#define IR_CHECKSUM_LEN		4

#define IR_CHECKSUM_START	0b111

#define IR_STAGE_IDLE 		0
#define IR_STAGE_START		1
#define IR_STAGE_LENGTH		2
#define IR_STAGE_DATA		3
#define IR_STAGE_CHECKSUM	4

namespace IR {
	extern uint8_t mLen;
	extern uint32_t message;

	void init(volatile uint8_t *PORT, uint8_t pin, IR_LED *led, void (*received_funct)());

	void receive();
	void update();

	bool send_8(uint8_t d);
}


#endif /* LIBCODE_COMMUNICATION_IR_IR_HANDLER_H_ */
