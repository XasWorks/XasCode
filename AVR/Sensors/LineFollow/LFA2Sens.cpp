
#include "LFA2Sens.h"

namespace LF {

	LFA2Sens::LFA2Sens(uint8_t const pin) : pin(pin) {
	}

	void LFA2Sens::update() {
		ADC_Lib::start_measurement(pin);
		ADC_Lib::start_measurement(pin +1);
		updating = 2;
	}

	void LFA2Sens::ADCUpdate() {
		if(ADC_Lib::measuredPin == pin) {
			updating--;
			readings[0] = ADC_Lib::lastResult;
		}
		else if(ADC_Lib::measuredPin == pin+1) {
			updating--;
			readings[1] = ADC_Lib::lastResult;
		}

		if(updating == 0) {
			uint16_t totalReading = readings[0] + readings[1];
			if(totalReading >= READING_THRESHOLD) {
				this->lineStatus = OK;
				this->lineOffset = (uint8_t)(((uint32_t)(readings[0] - readings[1]) * 127) / totalReading);
			}
			else {
				this->lineStatus = LOST;
			}
		}
	}

	bool LFA2Sens::isUpdated() {
		return this->updating == 0;
	}
}
