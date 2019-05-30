/*
 * Timer2.h
 *
 *  Created on: 21.06.2016
 *      Author: xasin
 */

#ifndef LIBCODE_TIMER_TIMER2_H_
#define LIBCODE_TIMER_TIMER2_H_

#include "TimerDefs.h"

#ifdef TIMER2_ENABLED

#include <avr/io.h>

#define TIMER2_PRESC_OFF 	0b000
#define TIMER2_PRESC_1		0b001
#define TIMER2_PRESC_8		0b010
#define TIMER2_PRESC_32		0b011
#define TIMER2_PRESC_64		0b100
#define TIMER2_PRESC_128	0b101
#define TIMER2_PRESC_256	0b110
#define TIMER2_PRESC_1024	0b111

#define TIMER2_MODE_TOP_OCR2A	1
#define TIMER2_MODE_FPWM_OCR2A	2

#define TIMER2_OC2A_OFF		0b00000000
#define TIMER2_OC2A_TOGGLE	0b01000000
#define TIMER2_OC2A_CLEAR	0b10000000
#define TIMER2_OC2A_SET		0b11000000

#define TIMER2_OC2B_OFF				0
#define TIMER2_OC2B_NPWM_TOGGLE		0b00010000
#define TIMER2_OC2B_FPWM_NONINV		0b00100000

namespace Timer2 {
	void set_prescaler(uint8_t presc);
	void set_OCR2A(uint8_t value);
	void set_OCR2B(uint8_t value);

	void set_mode(uint8_t mode);

	void set_OC2A_mode(uint8_t mode);
	void set_OC2B_mode(uint8_t mode);

	void set_OC2A_frequency(uint16_t freq, bool change_oc2a = false);
}



#endif
#endif /* LIBCODE_TIMER_TIMER2_H_ */
