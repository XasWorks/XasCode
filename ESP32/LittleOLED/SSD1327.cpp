/*
 * SSD1327.cpp
 *
 *  Created on: 29 Nov 2018
 *      Author: xasin
 */

#include "SSD1327.h"

namespace Peripheral {
namespace OLED {

#include "fonts/font-5x8.c"
#include "fonts/font-6x8.c"
#include "fonts/font-7x9.c"

void SSD1327::call_raw_update(void *args) {
	puts("Started updater thread!");

	uint32_t dummy;
	while(true) {
		xTaskNotifyWait(0, 0, &dummy, 2000/portTICK_PERIOD_MS);

		reinterpret_cast<SSD1327 *>(args)->raw_update();

		vTaskDelay(100/portTICK_PERIOD_MS);
	}
}

SSD1327::SSD1327() :
		DrawBox(128, 128),
		currentAction(nullptr), cmdBuffer(),
		screenBuffer(),
		updateTask(nullptr) {
}

void SSD1327::initialize() {
	start_cmd_set();

	send_cmd(ENABLE_LIN_GREYSCALE);
	send_cmd(DISPLAY_ENABLE);

	end_cmd_set();

	push_entire_screen();

	xTaskCreate(SSD1327::call_raw_update, "SSD1327 Updater", 2048, this, 3, &updateTask);
	puts("SSD1327 initialized!");
}

XaI2C::MasterAction* SSD1327::start_cmd_set() {
	assert(currentAction == nullptr);
	this->currentAction = new XaI2C::MasterAction(0b0111101);

	return currentAction;
}

void SSD1327::send_cmd(uint8_t cmdByte) {
	assert(currentAction != nullptr);

	cmdBuffer.push_back(cmdByte);
}
void SSD1327::send_cmd(uint8_t cmdByte, uint8_t param) {
	assert(currentAction != nullptr);

	cmdBuffer.push_back(cmdByte);
	cmdBuffer.push_back(param);
}

void SSD1327::end_cmd_set() {
	assert(currentAction != nullptr);

	currentAction->write(0x00, cmdBuffer.data(), cmdBuffer.size());

	auto ret = currentAction->execute();

	ESP_ERROR_CHECK(ret);

	cmdBuffer.clear();
	delete currentAction;
	currentAction = nullptr;
}

void SSD1327::data_write(void *data, size_t length) {
	assert(currentAction == nullptr);

	currentAction = new XaI2C::MasterAction(0b0111101);

	currentAction->write(DATA_STREAM, data, length);
	currentAction->execute();

	delete currentAction;
	currentAction = nullptr;
}

void SSD1327::set_coordinates(uint8_t column, uint8_t page, uint8_t maxColumn, uint8_t maxPage) {
	this->currentAction = start_cmd_set();

	const char oData[] = {SET_COLUMN_RANGE, column, maxColumn, SET_ROW_RANGE, page, maxPage};
	for(uint8_t i=0; i<6; i++)
		cmdBuffer.push_back(oData[i]);

	end_cmd_set();
}

void SSD1327::clear() {
	for(auto &page : screenBuffer) {
		for(uint8_t i=0; i<page.size(); i++)
			page[i] = 0;
	}
}
void SSD1327::push_entire_screen() {
	set_coordinates();

	for(uint8_t dataSet=0; dataSet<8; dataSet++) {

		uint16_t gScalePos = 0;
		for(uint8_t row=dataSet*16; row < (dataSet+1)*16; row++) {
			for(uint8_t page=0; page < 16; page++) {
				uint8_t displayByte = screenBuffer[page][row];

				for(uint8_t bit = 0; bit < 8; bit+=2) {
					greyscaleBuffer[gScalePos] = 0;
					if(displayByte & 0b01)
						greyscaleBuffer[gScalePos] |= 0b1111;
					if(displayByte & 0b10)
						greyscaleBuffer[gScalePos] |= 0b1111<<4;

					gScalePos++;
					displayByte >>= 2;
				}
			}
		}

		data_write(greyscaleBuffer.begin(), greyscaleBuffer.size());
	}
}

void SSD1327::request_redraw() {
	xTaskNotifyFromISR(updateTask, 0, eNoAction, nullptr);
}

void SSD1327::raw_update() {
	this->clear();
	this->redraw();
	this->push_entire_screen();
}

void SSD1327::set_pixel(int x, int y, bool on) {
	if(x < 0)
		return;
	if(y < 0)
		return;

	uint8_t page   = x / 8;
	uint8_t page_x = x - page*8;

	if(page >= screenBuffer.size())
		return;
	if(y >= screenBuffer[page].size())
		return;

	uint8_t &dByte = screenBuffer[page][y];

	if(on)
		dByte |= 1<<page_x;
	else
		dByte &= ~(1<<page_x);
}

} /* namespace OLED */
} /* namespace Peripheral */
