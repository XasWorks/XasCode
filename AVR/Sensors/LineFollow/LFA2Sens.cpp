
#include "LFA2Sens.h"

namespace LF {

	ASens2::ASens2(uint8_t const pin) : pin(pin), sigBuffer(0) {
	}

	void ASens2::update() {
		ADC_Lib::start_measurement(pin + 1);
		ADC_Lib::start_measurement(pin + 2);
		updating = 2;
	}

	void ASens2::ADCUpdate() {
		if(ADC_Lib::measuredPin == pin+1) {
			updating--;
			readings[0] = ADC_Lib::lastResult;
		}
		else if(ADC_Lib::measuredPin == pin+2) {
			updating--;
			readings[1] = ADC_Lib::lastResult;
		}


		if(updating == 0) {
			uint16_t totalReading = readings[0] + readings[1];
			this->lineStatus = OK;

			if(sigBuffer < 0)
				sigBuffer++;
			else if(sigBuffer > 0)
				sigBuffer--;

			if((PINA & 1<<pin) != 0)
				this->sigBuffer = INTSEC_BUFFER_TICKS;
			else if((PINA & 1 << (pin+3)) != 0)
				this->sigBuffer = -INTSEC_BUFFER_TICKS;

			if((PINA & (0b1001 << pin)) == (0b1001 << pin))
				this->lineStatus = INTSEC;
			else if(totalReading >= READING_THRESHOLD)
				this->lineOffset = (int8_t)((((int32_t)readings[0] - readings[1]) * 127) / totalReading);
			else {
				this->lineStatus = LOST;

				if(sigBuffer < 0)
					this->lineOffset = -127;
				else if(sigBuffer > 0)
					this->lineOffset = 127;
			}
		}
	}

	bool ASens2::isUpdated() {
		return this->updating == 0;
	}
}
