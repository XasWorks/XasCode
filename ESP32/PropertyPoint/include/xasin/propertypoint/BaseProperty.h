/*
 * BaseProperty.h
 *
 *  Created on: 3 Aug 2019
 *      Author: xasin
 */

#ifndef ESP32_PROPERTYPOINT_BASEPROPERTY_H_
#define ESP32_PROPERTYPOINT_BASEPROPERTY_H_

#include "cJSON.h"

#include <string>
#include <stdint.h>

namespace Xasin {
namespace PropP {

class PropertyHandler;

class BaseProperty {
protected:
	friend PropertyHandler;

	uint32_t last_modified_rev;

	PropertyHandler &handler;

	void update_me();

public:
	const std::string name;
	const uint16_t    simple_id;

	BaseProperty(PropertyHandler &handler, std::string name, uint16_t simple_id = 0);
	virtual ~BaseProperty();

	/*!	\brief Get any potential metadata, i.e. description, min/max values, type
	 *
	 * 	Not exactly necessary for all properties, but in the future this might allow
	 * 	dynamic clients to automatically display properties purely based on metadata
	 */
	virtual cJSON *get_metadata();

	/*!	\brief Get the cJSON representation of this property
	 * The parameter "from" specifies the revision from which on changes shall be sent
	 *	\attention The returned cJSON object must be deleted by the calling function!
	 */
	virtual cJSON *get_cJSON(uint32_t from);

	//! \brief Provide the property a cJSON object to change it
	virtual void from_cJSON(const cJSON *obj);

	//! \brief Return the last revision in which this property was modified
	uint32_t get_last_modified();

	//! \brief Check for any changes, and if so, update
	virtual void update();
};

} /* namespace PropP */
} /* namespace Xasin */

#endif /* ESP32_PROPERTYPOINT_BASEPROPERTY_H_ */
