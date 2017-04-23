/*
 * Timer1.cpp
 *
 *  Created on: 11.04.2016
 *      Author: xasin
 */

#include "Timer1.h"

#ifdef TIMER1_ENABLED

namespace Timer1 {
	void set_prescaler(uint8_t prescValue) {
		TCCR1B &= ~(0b00000111);
		TCCR1B |= prescValue;
	}

	void set_OCR1A(uint16_t value) {
		OCR1A = value;
	}

	void set_mode(uint8_t mode) {
		switch(mode) {
		case TIMER1_MODE_CTC:
			TCCR1B |= (1 << WGM12);
			TINTMASK |= (1 << OCIE1A);
		break;

		case TIMER1_MODE_FPWM:
			// Enable OC1A/B (non-inverting), set TOP to ICR1
			TCCR1A |= (1<< COM1A1 | 1<< COM1B1 | 1<< WGM11);
			TCCR1B |= (1<< WGM13 | 1<< WGM12);
		break;
		}
	}


	const uint16_t prescs[5] = {1, 8, 64, 256, 1024};
	void enable_CTC(uint16_t frequency) {
		set_mode(TIMER1_MODE_CTC);

		uint32_t CPU_Ticks = F_CPU / frequency;

		for(uint8_t i=0; i<5; i++) {
			if(CPU_Ticks < (65536*(uint32_t)prescs[i])) {

				set_prescaler(i + 1);
				set_OCR1A(CPU_Ticks/prescs[i] - 1);

				break;
			}
		}

		sei();
	}
}

#endif
