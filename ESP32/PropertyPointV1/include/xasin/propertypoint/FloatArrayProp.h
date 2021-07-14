/*
 * FloatArrayProp.h
 *
 *  Created on: 4 Aug 2019
 *      Author: xasin
 */

#ifndef ESP32_PROPERTYPOINT_FLOATARRAYPROP_H_
#define ESP32_PROPERTYPOINT_FLOATARRAYPROP_H_

#include "xasin/propertypoint/BaseProperty.h"

#include <vector>
#include <string>

#include <functional>

namespace Xasin {
namespace PropP {


class FloatArrayProp: public BaseProperty {
private:
	std::vector<float> old_numbers;
	float *new_numbers;
	const int length;

	const char * const* name_list;

public:
	FloatArrayProp(PropertyHandler &handler, std::string name,
			float *fltStart, int length = 1,
			const char *const *nameList = nullptr);
	virtual ~FloatArrayProp();

	std::function<void(const std::vector<float>&)> on_write;

	cJSON  *get_cJSON(uint32_t from);
	void 	from_cJSON(const cJSON *obj);

	void update();
};

} /* namespace PropP */
} /* namespace Xasin */

#endif /* ESP32_PROPERTYPOINT_FLOATARRAYPROP_H_ */
