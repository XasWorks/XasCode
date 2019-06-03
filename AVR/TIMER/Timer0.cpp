/*
 * Timer0.cpp
 *
 *  Created on: May 10, 2016
 *      Author: xasin
 */


#include "Timer0.h"

#ifdef TIMER0_ENABLED

namespace Timer0 {
	void set_prescaler(uint8_t presc) {
		TCCR0B = presc;
	}
	void set_OCR0A(uint8_t value) {
		OCR0A = value;
	}

	void set_mode(uint8_t mode) {
		switch(mode) {
		case TIMER0_MODE_CTC:
			TIMSK0 |= (1<< OCIE0A);
		case TIMER0_MODE_FREQ:
			TCCR0A |= (1<< WGM01);
		break;

		case TIMER0_MODE_40KHZ:
			TCCR0A |= (1<<WGM01);

			TCCR0B = TIMER0_PRESC_1;
			OCR0A  = F_CPU/80000 - 1;
		break;
		}
	}

	void set_OCA0_mode(uint8_t mode) {
		TCCR0A &= ~(0b1100000);
		TCCR0A |= mode;
	}
}

#endif
