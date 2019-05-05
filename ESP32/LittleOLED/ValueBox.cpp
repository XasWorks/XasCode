/*
 * ValueBox.cpp
 *
 *  Created on: 1 Apr 2019
 *      Author: xasin
 */

#include "ValueBox.h"
#include <cstdio>

namespace Peripheral {
namespace OLED {

ValueBox::ValueBox(int width, int height, DrawBox *topBox)
	: DrawBox(width, height, topBox),
	  value(width-17, height-1, this), unit(16, height-1, this),
	  postfix(""),
	  fillNum(0), oldFillNum(-1),
	  currentValue(0), valueMin(0), valueMax(0)
	  {

	value.set_centering(true, true);
	value.offsetY = 1;

	unit.offsetX = width-17;
	unit.offsetY = 1;
	unit.set_centering(true, true);
	unit.foreground = 0;

	request_redraw();
}

void ValueBox::redraw() {
	draw_box(1, 1, fillNum, height-2, 1, 1);
	draw_box(width-17, 1, 17, height-2, 2, 2);

	draw_line(0, 1, height-2, 1);
	draw_line(1, height-1, width-2, 0);
	draw_line(1, 0, width-2, 0);
	draw_line(width-1, 1, height-2, 1);

	DrawBox::redraw();

	if(oldFillNum == -1)
		mark_dirty_area({0, width-1, 0, height-1});
	else if(fillNum != oldFillNum)
		mark_dirty_area({fillNum+1, oldFillNum+1, 1, height-1});

	oldFillNum = fillNum;
}

void ValueBox::set_value(float val) {
	char buffer[30] = {};
	sprintf(buffer, "%d", int(val));

	value.set(buffer + postfix);

	float valPercent = val - valueMin;
	valPercent /= valueMax - valueMin;

	if(valPercent < 0)
		fillNum = 0;
	else if(valPercent > 1)
		fillNum = width-17;
	else
		fillNum = (width-17)*valPercent;

	request_redraw();
}

void ValueBox::setup(float minValue, float maxValue, const std::string unit, const std::string postfix) {
	valueMin = minValue;
	valueMax = maxValue;

	this->postfix = postfix;
	this->unit.set(unit);

	set_value(currentValue);
}

} /* namespace OLED */
} /* namespace Peripheral */
