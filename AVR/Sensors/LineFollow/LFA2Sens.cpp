
#include "LFA2Sens.h"

namespace LF {

	ASens2::ASens2(uint8_t const aPin, uint8_t volatile * const ePINx, uint8_t const ePin) :
		ePINx(ePINx), ePin(ePin), aPin(aPin) {
	}

	void ASens2::update() {
		ADC_Lib::start_measurement(aPin);
		ADC_Lib::start_measurement(aPin + 1);
		updating = 2;
	}

	void ASens2::ADCReadin() {
		if(ADC_Lib::measuredPin == aPin) {
			updating--;
			readings[0] = ADC_Lib::lastResult;
		}
		else if(ADC_Lib::measuredPin == aPin+1) {
			updating--;
			readings[1] = ADC_Lib::lastResult;
		}
	}

	void ASens2::NormalLF() {
		uint16_t totalReading = readings[0] + readings[1];

		if(totalReading >= READING_THRESHOLD) {
			this->lineStatus = OK;
			this->lineOffset = (int8_t)((((int32_t)readings[0] - readings[1]) * 127) / totalReading);
		}
		else
			this->lineStatus = LOST;
	}

	void ASens2::ADCUpdate() {
		this->ADCReadin();

		if(updating == 0) {
			uint8_t eSensors = (*ePINx >> ePin) & (0b11);

			if(eSensors == 0b11) {
				this->lineStatus = INTSEC;
				this->lineOffset = 0;
			}
			else {
				this->NormalLF();

				if(this->lineStatus != OK) {
					switch(eSensors) {
						case 0b01:
							this->lineOffset = 127;
						break;
						case 0b10:
							this->lineOffset = -127;
						break;
					}
				}
				else {
					if(this->lineOffset > INTSEC_DISABLE_THRESHOLD)
						eSensors &= ~(0b01);
					else if(this->lineOffset < -INTSEC_DISABLE_THRESHOLD)
						eSensors &= ~(0b10);

					switch(eSensors) {
					case 0b01:
						this->lineStatus = INTSEC;
						this->lineOffset = 127;
					break;
					case 0b10:
						this->lineStatus = INTSEC;
						this->lineOffset = -127;
					break;
					}
				}
			}
		}
	}

	bool ASens2::isUpdated() {
		return this->updating == 0;
	}
}
