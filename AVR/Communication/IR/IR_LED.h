/*
 * IRLED.h
 *
 *  Created on: 11.05.2016
 *      Author: xasin
 */

#ifndef LIBCODE_COMMUNICATION_IR_IR_LED_H_
#define LIBCODE_COMMUNICATION_IR_IR_LED_H_

class IR_LED {
public:
	IR_LED();

	virtual void on();
	virtual void off();
};

#endif /* LIBCODE_COMMUNICATION_IR_IR_LED_H_ */
