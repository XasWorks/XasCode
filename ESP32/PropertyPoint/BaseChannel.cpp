/*
 * ChannelBase.cpp
 *
 *  Created on: 4 Aug 2019
 *      Author: xasin
 */

#include "xasin/propertypoint/BaseChannel.h"
#include "xasin/propertypoint/PropertyHandler.h"

namespace Xasin {
namespace PropP {

BaseChannel::BaseChannel(PropertyHandler &handler)
	: handler(handler) {

	handler.channels.push_back(this);
}

BaseChannel::~BaseChannel() {
}

void BaseChannel::update_property(BaseProperty *prop) {
}

void BaseChannel::advance_revision() {
	handler.advance_revision();
}

const std::vector<BaseProperty*> &BaseChannel::get_properties() {
	return handler.properties;
}

} /* namespace PropP */
} /* namespace Xasin */
