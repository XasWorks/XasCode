/*
 * QuadrPropeller.cpp
 *
 *  Created on: 21 Jul 2019
 *      Author: xasin
 */

#include "xasin/QuadrPropeller.h"

#include <math.h>

#define LOG_LOCAL_LEVEL ESP_LOG_INFO
#include "esp_log.h"

namespace Xasin {
namespace Drone {

QuadrPropeller::QuadrPropeller(const quad_fact_t factors, BLDCHandler &motors, uint8_t id)
	: rawMotors(motors), mID(id),
	  factors(factors) {

	lin_squared = pow(factors.lin, 2);
	max_thrust  = factors.cnst + factors.lin + factors.quadrat;
}

QuadrPropeller::~QuadrPropeller() {
}

float QuadrPropeller::get_newtons() {
	float val = 0;

	float mPWR = rawMotors.get_motor_power(mID);
	int reversed = (mPWR < 0) ? -1 : 1;
	mPWR = fabs(mPWR);

	val += factors.cnst;
	val += factors.lin * mPWR;
	val += factors.quadrat * pow(mPWR,2);

	return val * reversed;
}

void QuadrPropeller::set_newtons(float num) {
	int reversed = (num < 0) ? -1 : 1;
	num = fabs(num);

	if(num > max_thrust)
		num = max_thrust;

	const float a = factors.quadrat;
	const float b = factors.lin;
	const float c = factors.cnst - num;

	float val = -b + sqrt(lin_squared - 4*a*c);
	val /= (2*a);

	if(val < 0.01 || isnan(val))
		rawMotors.set_motor_power(mID, 0);
	else
		rawMotors.set_motor_power(mID, val * reversed);

	ESP_LOGV("MAIN", "Calculated pwr: %f", val);
}

float QuadrPropeller::get_scaling_factor(float pwr) {
	if(fabs(pwr) < max_thrust)
		return 1;

	return max_thrust/fabs(pwr);
}

}
} /* namespace Xasin */

