
#ifndef LF2_A_SENS_H
#define LF2_A_SENS_H

#include "LFBasic.h"
#include "../../ADC/ADC_Lib.h"

#define READING_THRESHOLD 480

#define INTSEC_BUFFER_TICKS 20

namespace LF {

	class ASens2 : public Basic {
	private:
		int8_t sigBuffer;

		uint8_t const pin;
		uint16_t readings[2] = {0, 0};

		volatile uint8_t updating = 0;

	public:
		ASens2(uint8_t const pin);

		void update();
		void ADCUpdate();

		bool isUpdated();
	};
}

#endif
