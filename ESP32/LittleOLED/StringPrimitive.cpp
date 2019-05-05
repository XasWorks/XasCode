/*
 * StringPrimitive.cpp
 *
 *  Created on: 28 Mar 2019
 *      Author: xasin
 */

#include "StringPrimitive.h"

#include <math.h>

namespace Peripheral {
namespace OLED {

StringPrimitive::StringPrimitive()
	: StringPrimitive(0, 0, nullptr) {

}

StringPrimitive::StringPrimitive(int width, int height, DrawBox *headBox)
	: DrawBox(width, height, headBox),
	  wasChanged(false),
	  oldString(""), hCenter(false), vCenter(false),
	  font(&DEFAULT_FONT),
	  foreground(3), background(-1),
	  newString("") {

}

void StringPrimitive::redraw() {
	if(wasChanged) {
		wasChanged = false;
		mark_dirty_area({0, width , 0, height});
	}
	else if(newString.length() != oldString.length()) {
		int charX = hCenter ? width/2 - newString.length()*font->width/2 : 0;
		int charY = vCenter ? height/2-font->height/2 : 0;

		mark_dirty_area({charX, int(charX + std::max(oldString.length(), newString.length())*font->width),
			charY, charY+font->height});
	}
	else {
		for(int i=0; i<newString.length(); i++) {
			int charX = i*font->width;
			if(hCenter)
				charX += width/2 - newString.length()*font->width/2;
			int charY = vCenter ? height/2-font->height/2 : 0;

			if(newString.at(i) != oldString.at(i))
				mark_dirty_area({charX, font->width+charX, charY, charY+font->height});
		}
	}

	int xPos = 0;
	if(hCenter)
		xPos = width/2 - newString.length()*font->width/2;

	int yPos = 0;
	if(vCenter)
		yPos = height/2 - font->height/2;

	this->write_string(xPos, yPos, newString, foreground, background);
	oldString = newString;
}

std::string StringPrimitive::get() {
	return newString;
}
void StringPrimitive::set(const std::string nextString, bool notify) {
	if(nextString == oldString)
		return;

	newString = nextString;

	if(notify)
		request_redraw();
}

void StringPrimitive::printf(const char *input, ...) {
	va_list valist;
	va_start(valist, input);

	char buffer[64] = {};
	int printedSize = vsprintf(buffer, input, valist);

	set(buffer);
}

void StringPrimitive::set_centering(bool hCenter, bool vCenter) {
	if(hCenter != this->hCenter)
		wasChanged = true;
	if(vCenter != this->vCenter)
		wasChanged = true;

	this->hCenter = hCenter;
	this->vCenter = vCenter;

	if(wasChanged)
		request_redraw();
}

} /* namespace OLED */
} /* namespace Peripheral */
