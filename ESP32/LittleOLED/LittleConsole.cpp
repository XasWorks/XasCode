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

	for(uint8_t i=0; i<currentLines.size(); i++) {
		auto &str = currentLines[i];
		str.set("");
		str.set_head(&display);
		str.width = display.width;
		str.height = 8;

		str.offsetY = 8*i;
	}

	updateMutex = xSemaphoreCreateMutex();

	printfBuffer = new char[255];
}

void LittleConsole::shift_lines() {
	for(uint8_t i=1; i<currentLines.size(); i++)
		currentLines[i-1].set(currentLines[i].get(), false);

	currentLines[currentLines.size()-1].newString = "";
}

void LittleConsole::put_string(const char *input, size_t length) {
	if(length == 0)
		length = strlen(input);

	if(length == 0)
		return;

	xSemaphoreTake(updateMutex, portMAX_DELAY);
	uint8_t nLines = currentLines.size()-1;

	while(length-- != 0) {
		if(lastCharWasNewline) {
			lastCharWasNewline = false;
			shift_lines();
		}
		if(*input == '\n') {
			lastCharWasNewline = true;
		}
		else if(*input == '\r') {
			currentLines[nLines].set("", false);
		}
		else if(currentLines[nLines].newString.size() < 18){
			currentLines[nLines].newString += *input;

			if(currentLines[nLines].newString.size() == 18)
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
