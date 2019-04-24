/*
 * BatteryIndicator.h
 *
 *  Created on: 19 Apr 2019
 *      Author: xasin
 */

#ifndef ESP32_LITTLEOLED_BATTERYINDICATOR_H_
#define ESP32_LITTLEOLED_BATTERYINDICATOR_H_

#include "DrawBox.h"

namespace Peripheral {
namespace OLED {

class BatteryIndicator : public DrawBox {
private:
	int oldPercentage;
	int newPercentage;

	void redraw();

public:
	BatteryIndicator(DrawBox *headBox = nullptr);

	void set_fill(int percentage);
};

} /* namespace OLED */
} /* namespace Peripheral */

#endif /* ESP32_LITTLEOLED_BATTERYINDICATOR_H_ */
