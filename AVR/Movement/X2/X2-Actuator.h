/*
 * X2-Actuator.h
 *
 *  Created on: 24.10.2016
 *      Author: xasin
 */

#ifndef AVR_MOVEMENT_X2_X2_ACTUATOR_H_
#define AVR_MOVEMENT_X2_X2_ACTUATOR_H_

namespace X2 {

class Actuator {
private:
	static Actuator *headActuator;
	Actuator *nextActuator;

public:
	Actuator();

	virtual void ISRStepBy(float x, float r);
	static void ISRStepAllBy(float x, float r);
};

} /* namespace X2 */

#endif /* AVR_MOVEMENT_X2_X2_ACTUATOR_H_ */
