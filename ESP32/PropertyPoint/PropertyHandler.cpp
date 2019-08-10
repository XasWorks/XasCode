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

} /* namespace PropP */
} /* namespace Xasin */
