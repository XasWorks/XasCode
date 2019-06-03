/*
 * Timer0.h
 *
 *  Created on: May 10, 2016
 *      Author: xasin
 */

#ifndef LIBCODE_TIMER_TIMER0_H_
#define LIBCODE_TIMER_TIMER0_H_

#include "TimerDefs.h"

#ifdef TIMER0_ENABLED

#include <avr/io.h>

#define TIMER0_PRESC_OFF 	0b000
#define TIMER0_PRESC_1		0b001
#define TIMER0_PRESC_8		0b010
#define TIMER0_PRESC_64		0b011
#define TIMER0_PRESC_256	0b100
#define TIMER0_PRESC_1024	0b101

#define TIMER0_MODE_FREQ	1
#define TIMER0_MODE_CTC		0
#define TIMER0_MODE_40KHZ	2

#define TIMER0_OCA0_OFF		0b00000000
#define TIMER0_OCA0_TOGGLE	0b01000000
#define TIMER0_OCA0_CLEAR	0b10000000
#define TIMER0_OCA0_SET		0b11000000

namespace Timer0 {
	void set_prescaler(uint8_t presc);
	void set_OCR0A(uint8_t value);

	void set_mode(uint8_t mode);

	void set_OCA0_mode(uint8_t mode);
}

#endif
#endif /* LIBCODE_TIMER_TIMER0_H_ */
