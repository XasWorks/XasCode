/*
 * Control.h
 *
 *  Created on: 11 Sep 2018
 *      Author: xasin
 */


#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/touch_pad.h"

#include "soc/rtc.h"

#ifndef CONTROL_H_
#define CONTROL_H_

namespace Touch {

class Control {
private:
	const touch_pad_t padNo;

	uint64_t lastIntrTime;
	uint8_t  debounceCnt;

public:
	TaskHandle_t charDetectHandle;

	void char_detect_isr();
	void char_detect_task();

	Control(touch_pad_t padNo);

	uint16_t read_raw();
	bool touched();
};

} /* namespace Touch */

#endif /* CONTROL_H_ */
