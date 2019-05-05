/*
 * StringPrimitive.h
 *
 *  Created on: 28 Mar 2019
 *      Author: xasin
 */

#ifndef ESP32_LITTLEOLED_STRINGPRIMITIVE_H_
#define ESP32_LITTLEOLED_STRINGPRIMITIVE_H_

#include <stdarg.h>
#include <cstdio>
#include <string>


#include "DrawBox.h"

namespace Peripheral {
namespace OLED {

class StringPrimitive: public DrawBox {
private:
	bool wasChanged;

	std::string oldString;

	bool hCenter;
	bool vCenter;

	FontType *font;

protected:
	void redraw();

public:
	uint8_t foreground;
	uint8_t background;

	std::string newString;

	StringPrimitive();
	StringPrimitive(int width, int height, DrawBox *headBox);

	std::string get();
	void set(const std::string nextString, bool notify = true);
	void printf(const char *input, ...);

	void set_centering(bool hCenter, bool vCenter);
};

} /* namespace OLED */
} /* namespace Peripheral */

#endif /* ESP32_LITTLEOLED_STRINGPRIMITIVE_H_ */
