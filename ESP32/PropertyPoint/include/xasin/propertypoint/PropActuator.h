/*
 * PropActuator.h
 *
 *  Created on: 8 Aug 2019
 *      Author: xasin
 */

#ifndef ESP32_PROPERTYPOINT_PROPACTUATOR_H_
#define ESP32_PROPERTYPOINT_PROPACTUATOR_H_

#include "xasin/propertypoint/BaseProperty.h"

#include <array>
#include <functional>

#include <cmath>

namespace Xasin {
namespace PropP {

class PropActuator: public BaseProperty {
private:
	std::array<uint32_t, 5> mod_revs;
	std::array<float, 4> 	act_values;

	std::string				state;

public:
	enum actuator_prop_t {
		TARGET,
		IS,
		SPEED,
		ACCELLERATION
	};

	const float min_val;
	const float max_val;
	const std::string type;

	PropActuator(PropertyHandler &handler, std::string name,
			std::string type, float min_val = nanf(""), float max_val = nanf(""));
	virtual ~PropActuator();

	cJSON *get_cJSON(uint32_t from);
	void   from_cJSON(const cJSON *obj);

	void set_value(actuator_prop_t id, float newVal);
	float get_value(actuator_prop_t id);

	std::function<void (actuator_prop_t, float)> on_prop_write;
};

} /* namespace PropP */
} /* namespace Xasin */

#endif /* ESP32_PROPERTYPOINT_PROPACTUATOR_H_ */
