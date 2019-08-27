/*
 * PropActuator.cpp
 *
 *  Created on: 8 Aug 2019
 *      Author: xasin
 */

#include "xasin/propertypoint/PropActuator.h"
#include "xasin/propertypoint/PropertyHandler.h"

namespace Xasin {
namespace PropP {

PropActuator::PropActuator(PropertyHandler &handler, std::string name,
		std::string type, float min_v, float max_v)
		: 	BaseProperty(handler, name),
			mod_revs(), act_values(), state("OK"),
			min_val(min_v), max_val(max_v), type(type),
			on_prop_write(nullptr) {

}

PropActuator::~PropActuator() {
}

cJSON *PropActuator::get_cJSON(uint32_t from) {
	if(from > last_modified_rev)
		return cJSON_CreateNull();

	cJSON *obj = cJSON_CreateObject();

	if(from == 0) {
		if(!std::isnan(min_val))
			cJSON_AddNumberToObject(obj, "min", min_val);
		if(!std::isnan(max_val))
			cJSON_AddNumberToObject(obj, "max", max_val);
		cJSON_AddStringToObject(obj, "type", type.data());
	}

	const std::string valueNames[4] =
		{"tgt", "is", "speed", "accel"};

	for(int i=0; i<4; i++) {
		if(mod_revs[i] < from)
			continue;
		cJSON_AddNumberToObject(obj, valueNames[i].data(), act_values[i]);
	}
	if(mod_revs[4] >= from) {
		cJSON_AddStringToObject(obj, "state", state.data());
	}

	return obj;
}

void PropActuator::from_cJSON(const cJSON *obj) {
	if(on_prop_write == nullptr)
		return;

	if(!cJSON_IsObject(obj))
		return;

	const std::string valueNames[4] =
		{"tgt", "is", "speed", "accel"};

	for(int i=0; i<4; i++) {
		cJSON *propObj = cJSON_GetObjectItem(obj, valueNames[i].data());
		if(cJSON_IsNumber(propObj))
			on_prop_write(static_cast<actuator_prop_t>(i), propObj->valuedouble);
	}
}

void PropActuator::set_value(actuator_prop_t id, float newVal) {
	int i = static_cast<int>(id);

	if(newVal == act_values[i])
		return;

	act_values[i] = newVal;
	mod_revs[i] = handler.get_mod_revision();

	last_modified_rev = handler.get_mod_revision();
}

} /* namespace PropP */
} /* namespace Xasin */
