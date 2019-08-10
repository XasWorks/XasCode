/*
 * PropThrustMotor.cpp
 *
 *  Created on: 7 Aug 2019
 *      Author: xasin
 */

#include "xasin/propertypoint/PropThrustMotor.h"
#include "xasin/propertypoint/PropertyHandler.h"

namespace Xasin {
namespace PropP {

PropThrustMotor::PropThrustMotor(PropertyHandler &handler, std::string name,
		int length, const std::vector<std::string> nameList) :
		BaseProperty(handler, name),
		motorData(length), length(length), nameList(nameList) {
}

PropThrustMotor::~PropThrustMotor() {
}

cJSON *PropThrustMotor::get_cJSON(uint32_t from) {
	if(last_modified_rev < from)
		return cJSON_CreateNull();

	cJSON *outObj = cJSON_CreateObject();

	for(uint8_t i=0; i<length; i++) {
		const auto mData = motorData[i];
		if((mData.motor_tgt_rev < from) && (mData.motor_is_rev < from))
			continue;

		cJSON *mtrObj = cJSON_CreateObject();
		if(mData.motor_is_rev >= from)
			cJSON_AddNumberToObject(mtrObj, "is", mData.motor_is);
		if(mData.motor_tgt_rev >= from)
			cJSON_AddNumberToObject(mtrObj, "tgt", mData.motor_tgt);

		cJSON_AddItemToObject(outObj, nameList[i].data(), mtrObj);
	}

	return outObj;
}

void PropThrustMotor::set_motor_is(int id, float newtons) {
	if(id < 0 || id > length)
		return;

	if(newtons == motorData[id].motor_is)
		return;

	motorData[id].motor_is = newtons;
	motorData[id].motor_is_rev = handler.get_mod_revision();
}

void PropThrustMotor::set_motor_tgt(int id, float newtons) {
	if(id < 0 || id > length)
		return;
	if(newtons == motorData[id].motor_tgt)
		return;

	motorData[id].motor_tgt = newtons;
	motorData[id].motor_tgt_rev = handler.get_mod_revision();

	if(on_motor_set)
		on_motor_set(id, newtons);
}

void PropThrustMotor::from_cJSON(const cJSON *obj) {
	if(!cJSON_IsObject(obj))
		return;

	for(int i=0; i<length; i++) {
		auto name = nameList[i];

		const cJSON *numObj = cJSON_GetObjectItem(obj, name.data());
		if(cJSON_IsNumber(numObj)) {
			set_motor_tgt(i, numObj->valuedouble);
		}
	}
}

} /* namespace HTTP */
} /* namespace Xasin */
