/*
 * ValueBox.cpp
 *
 *  Created on: 1 Apr 2019
 *      Author: xasin
 */

#include "ValueBox.h"

namespace Peripheral {
namespace OLED {

ValueBox::ValueBox(int width, int height, DrawBox *topBox)
	: DrawBox(width, height, topBox),
	  value(width-20, height, topBox), unit(20, height, topBox),
	  oldValuePercent(0), valuePercent(0),
	  autoMode(false),
	  unit_str(""), currentValue(0), valueMin(0), valueMax(0)
	  {

	value.set_centering(true, true);

	unit.offsetX = width-20;
	unit.set_centering(false, true);
}

} /* namespace OLED */
} /* namespace Peripheral */
