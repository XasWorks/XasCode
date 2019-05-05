/*
 * LittleConsole.cpp
 *
 *  Created on: 30 Nov 2018
 *      Author: xasin
 */

#include "LittleConsole.h"

#include <cmath>

namespace Peripheral {
namespace OLED {

LittleConsole::LittleConsole(DrawBox &display)
	: display(display),
	  currentLines(), lineShift(0), lastCharWasNewline(false) {

	for(uint8_t i=0; i<currentLines.size(); i++) {
		auto &str = currentLines[i];
		str.set("");
		str.set_head(&display);

		str.width = display.width;
		str.height = 8;

		str.offsetY = display.height - 8*(i+1);
	}

	updateMutex = xSemaphoreCreateMutex();

	printfBuffer = new char[255];
}

void LittleConsole::shift_lines() {
	for(uint8_t i=currentLines.size()-1; i!=0; i--)
		currentLines[i].set(currentLines[i-1].get(), false);

	currentLines[0].newString = "";
}

void LittleConsole::put_string(const char *input, size_t length) {
	if(length == 0)
		length = strlen(input);

	if(length == 0)
		return;

	xSemaphoreTake(updateMutex, portMAX_DELAY);
	while(length-- != 0) {
		if(lastCharWasNewline) {
			lastCharWasNewline = false;
			shift_lines();
		}
		if(*input == '\n') {
			lastCharWasNewline = true;
		}
		else if(*input == '\r') {
			currentLines[0].set("", false);
		}
		else if(currentLines[0].newString.size() < currentLines[0].get_line_width()){
			currentLines[0].newString += *input;

			if(currentLines[0].newString.size() == currentLines[0].get_line_width())
				lastCharWasNewline = true;
		}

		input++;
	}
	xSemaphoreGive(updateMutex);

	display.request_redraw();
}

int LittleConsole::vprintf(const char *input, va_list args) {
	int printedSize = vsprintf(printfBuffer, input, args);
	put_string(printfBuffer, printedSize);

	return printedSize;
}
void LittleConsole::printf(const char *input, ...) {
	va_list valist;
	va_start(valist, input);

	this->vprintf(input, valist);
	va_end(valist);
}

void LittleConsole::printf_style(const char *input, ...) {
	va_list valist;
	va_start(valist, input);

	vsprintf(printfBuffer, input, valist);

	const char *c = printfBuffer;
	while(*c != '\0') {
		put_string(c++, 1);
		vTaskDelay(100);
	}
}

} /* namespace OLED */
} /* namespace Peripheral */
