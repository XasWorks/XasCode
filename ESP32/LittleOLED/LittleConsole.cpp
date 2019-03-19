/*
 * LittleConsole.cpp
 *
 *  Created on: 30 Nov 2018
 *      Author: xasin
 */

#include "LittleConsole.h"

namespace Peripheral {
namespace OLED {

LittleConsole::LittleConsole(DrawBox &display)
	: display(display),
	  currentLines(), lineShift(0), lastCharWasNewline(false) {

	display.onRedraw = [this]() {
		this->raw_update();
	};

	updateMutex = xSemaphoreCreateMutex();

	printfBuffer = new char[255];
}

void LittleConsole::raw_update() {
	xSemaphoreTake(updateMutex, portMAX_DELAY);
	for(uint8_t line=0; line<currentLines.size(); line++) {
		display.write_string(0, 8*line, currentLines[(line+lineShift)%currentLines.size()], line%4, 1);
	}
	xSemaphoreGive(updateMutex);
}

void LittleConsole::put_string(const char *input, size_t length) {
	if(length == 0)
		length = strlen(input);

	if(length == 0)
		return;

	xSemaphoreTake(updateMutex, portMAX_DELAY);
	uint8_t nLines = currentLines.size();

	while(length-- != 0) {
		if(lastCharWasNewline) {
			lastCharWasNewline = false;

			lineShift++;
			if(lineShift >= currentLines.size())
				lineShift = 0;

			currentLines[(nLines-1+lineShift)%nLines].clear();
		}
		if(*input == '\n') {
			lastCharWasNewline = true;
		}
		else if(*input == '\r') {
			currentLines[(nLines-1+lineShift)%nLines].clear();
		}
		else if(currentLines[(nLines-1+lineShift)%nLines].size() < 18){
			currentLines[(nLines-1+lineShift)%nLines] += *input;

			if(currentLines[(nLines-1+lineShift)%nLines].size() == 18)
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

} /* namespace OLED */
} /* namespace Peripheral */
