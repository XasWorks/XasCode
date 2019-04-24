/*
 * BatteryIndicator.cpp
 *
 *  Created on: 19 Apr 2019
 *      Author: xasin
 */

#include "BatteryIndicator.h"
#include <cstdio>

namespace Peripheral {
namespace OLED {

BatteryIndicator::BatteryIndicator(DrawBox *headBox)
	: DrawBox(9, 6, headBox),
	  oldPercentage(1000), newPercentage(0) {
}

void BatteryIndicator::redraw() {
	if(oldPercentage != newPercentage)
		mark_dirty_area({0, 8, 0, 5});

	oldPercentage = newPercentage;

	draw_line(0, 1, 4, 1);
	draw_box(1, 0, 8, 6);

	for(int i=0; i<24; i++) {
		if(i > oldPercentage)
			break;

		set_pixel(7 - i/4, 1 + i%4, 2);
	}
}

void BatteryIndicator::set_fill(int percentage) {
	percentage = percentage * 24 / 100;

	if(percentage == newPercentage)
		return;

	newPercentage = percentage;
	request_redraw();
}
} /* namespace OLED */
} /* namespace Peripheral */
