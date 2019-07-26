/*
 * QuadrPropeller.cpp
 *
 *  Created on: 21 Jul 2019
 *      Author: xasin
 */

#include "xasin/QuadrPropeller.h"

#include <math.h>

#include "esp_log.h"

namespace Xasin {

QuadrPropeller::QuadrPropeller(const quad_fact_t factors, OneShot125 &motors, uint8_t id)
	: rawMotors(motors), mID(id),
	  factors(factors) {

	lin_squared = pow(factors.lin, 2);
}

QuadrPropeller::~QuadrPropeller() {
}

float QuadrPropeller::get_newtons() {
	float val = 0;

	val += factors.cnst;
	val += factors.lin * rawMotors.get_real_power(mID);
	val += factors.quadrat * pow(rawMotors.get_real_power(mID),2);

	return val;
}

void QuadrPropeller::set_newtons(float num) {
	if(num > 0.5)
		num = 0.5;

	const float a = factors.quadrat;
	const float b = factors.lin;
	const float c = factors.cnst - num;

	float val = -b + sqrt(lin_squared - 4*a*c);
	val /= (2*a);

	if(val > 0.7)
		val = 0.7;

	if(val < 0.01 || isnan(val))
		rawMotors.set_motor_power(mID, 0);
	else
		rawMotors.set_motor_power(mID, val);

	ESP_LOGI("MAIN", "Calculated pwr: %f", val);
}

} /* namespace Xasin */
