/*
 * LF3Sens.h
 *
 *  Created on: Jan 3, 2016
 *      Author: xasin
 */

#ifndef CODE_LINEFOLLOW_LF3SENS_H_
#define CODE_LINEFOLLOW_LF3SENS_H_

#define INTSEC_DEFAULT_DIR LF_RIGHT
#define DELAY_VAL 1

#include <avr/io.h>
#include "LFBasic.h"

namespace LF {

class Sens3 : public Basic {
private:
	//Update delay value, to make sure the sensors can change properly
	uint16_t uDelay = 0;
	//Value of the old sensors, to detect changes
	uint8_t oSens = 0;

	//Update the sensors.
	void setStatus(uint8_t sensors);

	volatile uint8_t * const PINx;
	uint8_t const pins;

public:
	Sens3(volatile uint8_t *PINx, uint8_t pins);

	void update();
};

}

#endif /* CODE_LINEFOLLOW_LF3SENS_H_ */
