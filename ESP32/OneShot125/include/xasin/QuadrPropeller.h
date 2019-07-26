/*
 * QuadrPropeller.h
 *
 *  Created on: 21 Jul 2019
 *      Author: xasin
 */

#ifndef ESP32_ONESHOT125_QUADRPROPELLER_H_
#define ESP32_ONESHOT125_QUADRPROPELLER_H_

#include "xasin/OneShot125.h"

namespace Xasin {

struct quad_fact_t {
	float cnst;
	float lin;
	float quadrat;
};

class QuadrPropeller {
private:
	float lin_squared;

public:
	OneShot125 &rawMotors;
	const uint8_t mID;

	const quad_fact_t factors;

	QuadrPropeller(const quad_fact_t factors, OneShot125 &motors, uint8_t id);
	virtual ~QuadrPropeller();

	float get_newtons();
	void  set_newtons(float num);
};

} /* namespace Xasin */

#endif /* ESP32_ONESHOT125_QUADRPROPELLER_H_ */
