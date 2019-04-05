/*
 * ValueBox.h
 *
 *  Created on: 1 Apr 2019
 *      Author: xasin
 */

#ifndef ESP32_LITTLEOLED_VALUEBOX_H_
#define ESP32_LITTLEOLED_VALUEBOX_H_

#include "StringPrimitive.h"

namespace Peripheral {
namespace OLED {

class ValueBox : public DrawBox {
private:

	StringPrimitive value;
	StringPrimitive unit;

	uint8_t oldValuePercent;
	uint8_t valuePercent;

	bool autoMode;

	std::string unit_str;

	float currentValue;
	float valueMin;
	float valueMax;

protected:
	void redraw();

public:
	ValueBox(int width, int height, DrawBox *topBox);

	void enable_auto(float minValue, float maxValue, const std::string unit);

	void set_valString(const char *inString, ...);
	void set_valPercent(float value);

	void set_value(float value);

	void set_unit(const std::string unit);
};

} /* namespace OLED */
} /* namespace Peripheral */

#endif /* ESP32_LITTLEOLED_VALUEBOX_H_ */
