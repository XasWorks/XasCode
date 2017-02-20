
#ifndef LF2_A_SENS_H
#define LF2_A_SENS_H

#include "LFBasic.h"
#include "../../ADC/ADC_Lib.h"

#define READING_THRESHOLD 480

#define INTSEC_DISABLE_THRESHOLD 30

namespace LF {

	class ASens2 : public Basic {
	private:
		volatile uint8_t * const ePINx;
		uint8_t const ePin;
		uint8_t const aPin;
		uint16_t readings[2] = {0, 0};

		volatile uint8_t updating = 0;

		void ADCReadin();
		void NormalLF();

	public:
		ASens2(uint8_t const aPin, uint8_t volatile * const ePINx, uint8_t const ePin);

		void update();
		void ADCUpdate();

		bool isUpdated();
	};
}

#endif
