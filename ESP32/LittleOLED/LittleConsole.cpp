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
	  lineStrings(), currentLines(),
	  lineShift(0),
	  lastCharWasNewline(false), lastCharWasR(false) {

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

void LittleConsole::shift_g_lines() {
	for(uint8_t i=currentLines.size()-1; i!=0; i--)
		currentLines[i].set(currentLines[i-1].get(), false);

	currentLines[0].newString = "";
}
void LittleConsole::shift_s_lines() {
	for(uint8_t i=lineStrings.size()-1; i!=0; i--)
		lineStrings[i] = lineStrings[i-1];

	lineStrings[0] = "";
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
			shift_s_lines();
		}

		if(lastCharWasR) {
			lastCharWasR = false;
			lineStrings[0] = "";
		}

		if(*input == '\n') {
			lastCharWasNewline = true;
		}
		else if(*input == '\r') {
			lastCharWasR = true;
		}
		else
			lineStrings[0] += *input;

		input++;
	}

	int lineWidth = currentLines[0].get_line_width();
	int lastLinePosition = 0;

	for(int i=0; i < lineStrings.size(); i++) {
		auto str = lineStrings[i];

		int lineCount = (str.size()/lineWidth);
		for(int lNum = lineCount; lNum >= 0; lNum--) {
			currentLines[lastLinePosition++].set(str.substr(lNum*lineWidth, lineWidth), false);

			if(lastLinePosition >= currentLines.size())
				break;
		}

		if(lastLinePosition >= currentLines.size())
			break;
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
		vTaskDelay(30);
	}
}

} /* namespace OLED */
} /* namespace Peripheral */
