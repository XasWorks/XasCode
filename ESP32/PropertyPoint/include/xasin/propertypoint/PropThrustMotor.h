/*
 * PropThrustMotor.h
 *
 *  Created on: 7 Aug 2019
 *      Author: xasin
 */

#ifndef ESP32_PROPERTYPOINT_PROPTHRUSTMOTOR_H_
#define ESP32_PROPERTYPOINT_PROPTHRUSTMOTOR_H_

#include "xasin/propertypoint/BaseProperty.h"

#include <vector>
#include <string>

#include <functional>

namespace Xasin {
namespace PropP {

class PropThrustMotor: public PropP::BaseProperty {
	struct motor_data_t {
		float 		motor_tgt;
		uint32_t 	motor_tgt_rev;
		float 		motor_is;
		uint32_t	motor_is_rev;
	};

	std::vector<motor_data_t> motorData;

public:
	const int length;
	const std::vector<std::string> nameList;

	PropThrustMotor(PropertyHandler &handler, std::string name,
			int length, const std::vector<std::string> nameList);
	virtual ~PropThrustMotor();

	cJSON *get_cJSON(uint32_t from);
	void from_cJSON(const cJSON * obj);

	void set_motor_is(int id, float newtons);
	void set_motor_tgt(int id, float target);

	float get_motor_is(int id);
	float get_motor_tgt(int id);

	std::function<void (int, float)> on_motor_set;
};

} /* namespace HTTP */
} /* namespace Xasin */

#endif /* ESP32_PROPERTYPOINT_PROPTHRUSTMOTOR_H_ */
