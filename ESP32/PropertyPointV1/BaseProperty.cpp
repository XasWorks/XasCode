/*
 * BaseProperty.cpp
 *
 *  Created on: 3 Aug 2019
 *      Author: xasin
 */

#include "xasin/propertypoint/BaseProperty.h"
#include "xasin/propertypoint/PropertyHandler.h"

namespace Xasin {
namespace PropP {

BaseProperty::BaseProperty(PropertyHandler &handler, std::string name, uint16_t simple_id)
	: last_modified_rev(0), handler(handler), name(name), simple_id(simple_id) {

	handler.properties.push_back(this);
}

BaseProperty::~BaseProperty() {
}

void BaseProperty::update_me() {
	this->last_modified_rev = handler.get_mod_revision();
	handler.update_property(this);
}

cJSON *BaseProperty::get_metadata() {
	auto json = cJSON_CreateObject();

	cJSON_AddStringToObject(json, "name", name.data());

	return json;
}

cJSON *BaseProperty::get_cJSON(uint32_t from) {
	return cJSON_CreateNull();
}

void BaseProperty::from_cJSON(const cJSON *obj) {}


uint32_t BaseProperty::get_last_modified() {
	return last_modified_rev;
}

void BaseProperty::update() {}

} /* namespace PropP */
} /* namespace Xasin */
