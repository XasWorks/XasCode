/*
 * ShiftReg.h
 *
 *  Created on: 15.12.2016
 *      Author: xasin
 */

#ifndef AVR_INTERFACING_SHIFT_SHIFTREG_H_
#define AVR_INTERFACING_SHIFT_SHIFTREG_H_

#include <avr/io.h>
#include <util/delay.h>


namespace Interfacing {

#define SET_DATA	*PORT |= (0b1 << (pin));
#define UNSET_DATA	*PORT &= ~(0b1 << (pin));
#define SEND_RCK	*PORT |= (0b100 << (pin)); *PORT &= ~(0b100 << (pin));
#define SEND_SRCK 	*PORT |= (0b10 << (pin)); *PORT &= ~(0b10 << (pin));

class ShiftReg {
private:
	volatile uint8_t * const PORT;
	const uint8_t pin;

	void writeByte(uint8_t data);

public:
	void write8(uint8_t data);
	void write16(uint16_t data);
	void write32(uint32_t data);

	void write(void *data, uint8_t bytes);

	ShiftReg(volatile uint8_t * const PORT, const uint8_t pin);
};

} /* namespace Interfacing */

#endif /* AVR_INTERFACING_SHIFT_SHIFTREG_H_ */
