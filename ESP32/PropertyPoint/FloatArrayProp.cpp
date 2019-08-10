/*
 * FloatArrayProp.cpp
 *
 *  Created on: 4 Aug 2019
 *      Author: xasin
 */

#include "xasin/propertypoint/FloatArrayProp.h"
#include "xasin/propertypoint/PropertyHandler.h"
#include <math.h>

#define LOG_LOCAL_LEVEL ESP_LOG_INFO
#include "esp_log.h"

namespace Xasin {
namespace PropP {

FloatArrayProp::FloatArrayProp(PropertyHandler &handler, std::string name,
		float *fltStart, int length,
		const char *const *nameList)
	: BaseProperty(handler, name),
	  old_numbers(length), new_numbers(fltStart), length(length),
	  name_list(nameList),
	  on_write(nullptr) {

}

FloatArrayProp::~FloatArrayProp() {
}

cJSON *FloatArrayProp::get_cJSON(uint32_t from) {
	if(length == 1)
		return cJSON_CreateNumber(*new_numbers);
	else if(name_list == nullptr)
		return cJSON_CreateFloatArray(new_numbers, length);
	else {
		cJSON *obj = cJSON_CreateObject();

		for(int i=0; i<length; i++) {
			cJSON_AddNumberToObject(obj, name_list[i], new_numbers[i]);
		}

		return obj;
	}
}

void FloatArrayProp::from_cJSON(const cJSON *obj) {
	if(!on_write)
		return;

	ESP_LOGD("Prop::FLT", "Updating from JSON");

	std::vector<float> updated_numbers = std::vector<float>(length);
	for(int i=0; i<length; i++)
		updated_numbers[i] = new_numbers[i];

	if(length == 1) {
		if(!cJSON_IsNumber(obj))
			return;
		updated_numbers[0] = obj->valuedouble;
	}
	else if(name_list == nullptr) {
		if(!cJSON_IsArray(obj))
			return;

		int i=0;
		int remaining = fmin(length, cJSON_GetArraySize(obj));
		while(remaining--) {
			auto arrItem = cJSON_GetArrayItem(obj, i);
			if(cJSON_IsNumber(arrItem))
				updated_numbers[i] = arrItem->valuedouble;
		}
	}
	else {
		if(!cJSON_IsObject(obj))
			return;

		for(int i=0; i<length; i++) {
			const char *name = name_list[i];

			const cJSON *objItem = cJSON_GetObjectItem(obj, name);
			if(cJSON_IsNumber(objItem)) {
				ESP_LOGV("Prop::FLT", "Got update for %d (%f)", i, objItem->valuedouble);
				updated_numbers[i] = objItem->valuedouble;
			}
		}
	}

	on_write(updated_numbers);
}

void FloatArrayProp::update() {
	if(last_modified_rev == handler.get_mod_revision())
		return;

	bool mod_found = false;
	for(int i=0; i<length; i++) {
		if(old_numbers[i] != new_numbers[i]) {
			mod_found = true;
			old_numbers[i] = new_numbers[i];
		}
	}

	if(!mod_found)
		return;

	update_me();
}

} /* namespace PropP */
} /* namespace Xasin */
