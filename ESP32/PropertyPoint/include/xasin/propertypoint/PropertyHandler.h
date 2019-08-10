/*
 * PropertyHandler.h
 *
 *  Created on: 3 Aug 2019
 *      Author: xasin
 */

#ifndef ESP32_PROPERTYPOINT_PROPERTYHANDLER_H_
#define ESP32_PROPERTYPOINT_PROPERTYHANDLER_H_

#include <vector>

namespace Xasin {
namespace PropP {

class BaseChannel;
class BaseProperty;

class PropertyHandler {
protected:
	friend BaseProperty;
	friend BaseChannel;

	uint32_t mod_revision;

	std::vector<BaseProperty *> properties;
	std::vector<BaseChannel  *>	channels;

	void update_property(BaseProperty *prop);

	void advance_revision();

public:

	PropertyHandler();
	virtual ~PropertyHandler();

	uint32_t get_mod_revision();

	const std::vector<BaseProperty *> get_modified_since(uint32_t mod_id);
};

} /* namespace PropP */
} /* namespace Xasin */

#endif /* ESP32_PROPERTYPOINT_PROPERTYHANDLER_H_ */
