/*
 * Timer2.cpp
 *
 *  Created on: 21.06.2016
 *      Author: xasin
 */

#include "Timer2.h"

#ifdef TIMER2_ENABLED

namespace Timer2 {
	void set_prescaler(uint8_t presc) {
		TCCR2B &= ~(0b111);
		TCCR2B |= presc;
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

	const uint16_t prescs[7] = {1, 8, 32, 64, 128, 256, 1024};

	void set_OC2A_frequency(uint16_t freq, bool change_oc2a) {
		if(change_oc2a) {
			if(freq == 0) {
				set_OC2A_mode(TIMER2_OC2A_OFF);
				return;
			}
			else
				set_OC2A_mode(TIMER2_OC2A_TOGGLE);
		}

		if(freq == 0)
			freq = 1;

		uint32_t CPU_Ticks = F_CPU/2/freq;

		for(uint8_t i=0; i<7; i++) {
			if(CPU_Ticks < (255*(uint32_t)prescs[i])) {
				set_prescaler(i + 1);
				set_OCR2A(CPU_Ticks/prescs[i] - 1);

				break;
			}
		}
	}
}

#endif
