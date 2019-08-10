/*
 * ChannelBase.h
 *
 *  Created on: 4 Aug 2019
 *      Author: xasin
 */

#ifndef ESP32_PROPERTYPOINT_CHANNELBASE_H_
#define ESP32_PROPERTYPOINT_CHANNELBASE_H_

#include <vector>

namespace Xasin {
namespace PropP {

class BaseProperty;
class PropertyHandler;

class BaseChannel {
protected:
	friend PropertyHandler;

	PropertyHandler &handler;

	virtual void update_property(BaseProperty *prop);

	void advance_revision();
	const std::vector<BaseProperty*> &get_properties();

public:
	BaseChannel(PropertyHandler &handler);
	virtual ~BaseChannel();
};

} /* namespace PropP */
} /* namespace Xasin */

#endif /* ESP32_PROPERTYPOINT_CHANNELBASE_H_ */
