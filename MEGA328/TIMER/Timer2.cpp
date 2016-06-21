/*
 * Timer2.cpp
 *
 *  Created on: 21.06.2016
 *      Author: xasin
 */

#include "Timer2.h"

namespace Timer2 {
	void set_prescaler(uint8_t presc) {
		TCCR2B = presc;
	}
	void set_OCR2A(uint8_t value) {
		OCR2A = value;
	}
	void set_OCR2B(uint8_t value) {
		OCR2B = value;
	}

	void set_mode(uint8_t mode) {
		switch(mode) {
		case TIMER2_MODE_TOP_OCR2A:
			TCCR2A = (1<< WGM21);
		break;

		case TIMER2_MODE_FPWM_OCR2A:
			TCCR2A = (1<< WGM21 | 1<< WGM20);
			TCCR2B |= (1<< WGM22);
		}
	}

	void set_OC2A_mode(uint8_t mode) {
		TCCR2A &= ~(0b11000000);
		TCCR2A |= mode;
	}

	void set_OC2B_mode(uint8_t mode) {
		TCCR2A &= ~(0b11000);
		TCCR2A |= mode;
	}
}
