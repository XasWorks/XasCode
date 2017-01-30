/*
 * RGBStatus.h
 *
 *  Created on: 06.11.2016
 *      Author: xasin
 */

#ifndef AVR_COMMUNICATION_RGBSTATUS_H_
#define AVR_COMMUNICATION_RGBSTATUS_H_

#include <avr/io.h>
#include <util/delay.h>

namespace Communication {

enum Pattern : uint16_t {
	off			= 0,
	on			= 0b111111111111,
	slowblink	= 0b111,
	blink		= 0b111000111,
	quickblink	= 0b1100110011,
	flash		= 0b10101010101,
};

class RGBStatus {
private:
	uint16_t RMode = 0;
	uint16_t GMode = 0;
	uint16_t BMode = 0;

	const uint8_t pinR, pinG, pinB;
	volatile uint8_t * const PORT;

	volatile uint8_t sequencePosition = 0;

public:
	RGBStatus(volatile uint8_t *PORT, uint8_t pinR, uint8_t pinG, uint8_t pinB);

	void setModes(uint16_t rMode, uint16_t gMode, uint16_t bMode);
	void signal(uint16_t red, uint16_t green, uint16_t blue, uint8_t length);

	void update();
};

} /* namespace Communication */

#endif /* AVR_COMMUNICATION_RGBSTATUS_H_ */
