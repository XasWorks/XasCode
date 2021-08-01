/*
 * PropertyHandler.cpp
 *
 *  Created on: 3 Aug 2019
 *      Author: xasin
 */

#include "xasin/propertypoint/PropertyHandler.h"
#include "xasin/propertypoint/BaseProperty.h"
#include "xasin/propertypoint/BaseChannel.h"

namespace Xasin {
namespace PropP {

PropertyHandler::PropertyHandler()
	: mod_revision(0), properties(), channels() {
}

PropertyHandler::~PropertyHandler() {
}

void PropertyHandler::update_property(BaseProperty *prop) {
	for(auto c : channels)
		c->update_property(prop);
}

void PropertyHandler::advance_revision() {
	for(auto prop : properties) {
		if(prop->last_modified_rev >= mod_revision)
			mod_revision = prop->last_modified_rev+1;
	}
}

uint32_t PropertyHandler::get_mod_revision() {
	return mod_revision;
}

const std::vector<BaseProperty *> PropertyHandler::get_modified_since(uint32_t mod_id) {
	auto outVec = std::vector<BaseProperty *>();

	for(auto prop : properties) {
		prop->update();

		if(prop->last_modified_rev >= mod_id)
			outVec.push_back(prop);
	}

	return outVec;
}

cJSON *PropertyHandler::get_cJSON_since(uint32_t mod_id) {
	cJSON *data_obj = cJSON_CreateObject();

	auto to_send_buffer = get_modified_since(mod_id);
	if(to_send_buffer.size() > 0) {
		cJSON *mod_obj = cJSON_CreateObject();

		for(auto prop : to_send_buffer) {
			cJSON_AddItemToObject(mod_obj, prop->name.data(), prop->get_cJSON(mod_id));
		}

		cJSON_AddItemToObject(data_obj, "modified", mod_obj);

		this->advance_revision();
	}
	cJSON_AddNumberToObject(data_obj, "rev", mod_revision);

	return data_obj;
}

void PropertyHandler::feed_cJSON(const cJSON *json) {
	if(!cJSON_IsObject(json))
		return;

	for(auto prop : properties) {
		cJSON *fetchedObj = cJSON_GetObjectItem(json, prop->name.data());
		if(!cJSON_IsInvalid(fetchedObj)) {
			prop->from_cJSON(fetchedObj);
		}
	}
}

} /* namespace PropP */
} /* namespace Xasin */
