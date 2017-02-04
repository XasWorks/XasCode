
#ifndef LF2_A_SENS_H
#define LF2_A_SENS_H

#include "LFBasic.h"
#include "../../ADC/ADC_Lib.h"

#define READING_THRESHOLD 480

namespace LF {

	class LFA2Sens : Basic {
	private:
		uint8_t const pin;
		uint16_t readings[2] = {0, 0};

		volatile uint8_t updating = 0;

	public:
		LFA2Sens(uint8_t const pin);

		void update();
		void ADCUpdate();

		bool isUpdated();
	};
}

#endif
