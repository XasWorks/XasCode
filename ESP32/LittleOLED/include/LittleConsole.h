/*
 * LittleConsole.h
 *
 *  Created on: 30 Nov 2018
 *      Author: xasin
 */

#ifndef COMPONENTS_LITTLEOLED_LITTLECONSOLE_H_
#define COMPONENTS_LITTLEOLED_LITTLECONSOLE_H_

#include <cstdio>
#include <cstring>
#include <stdarg.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "SSD1306.h"

namespace Peripheral {
namespace OLED {

class LittleConsole {
private:
	static void callUpdate(void *args);

	char *printfBuffer;

	SSD1306 &display;

	std::array<std::string, 4> currentLines;
	uint8_t lineShift;
	bool lastCharWasNewline;

	TaskHandle_t updateTask;
	SemaphoreHandle_t updateMutex;

public:
	void raw_update();

	LittleConsole(SSD1306 &display);

	void update();

	void put_string(const char *input, size_t length = 0);
	int  vprintf(const char *format, va_list args);
	void printf(const char *input, ...);
};

} /* namespace OLED */
} /* namespace Peripheral */

#endif /* COMPONENTS_LITTLEOLED_LITTLECONSOLE_H_ */
