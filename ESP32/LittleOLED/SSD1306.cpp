/*
 * SSD1306.cpp
 *
 *  Created on: 29 Nov 2018
 *      Author: xasin
 */

#include "SSD1306.h"

namespace Peripheral {
namespace OLED {

#include "fonttype.h"

void SSD1306::call_raw_update(void *args) {
	puts("Started updater thread!");

	uint32_t dummy;
	while(true) {
		xTaskNotifyWait(0, 0, &dummy, 200/portTICK_PERIOD_MS);

		reinterpret_cast<SSD1306 *>(args)->raw_update();

		vTaskDelay(100/portTICK_PERIOD_MS);
	}
}

SSD1306::SSD1306() :
		DrawBox(128, 32),
		currentAction(nullptr), cmdBuffer(),
		screenBuffer(),
		updateTask(nullptr) {
}

void SSD1306::initialize() {
	start_cmd_set();

	send_cmd(SET_MUX_RATIO, 31);
	send_cmd(SET_DISPLAY_OFFSET, 0);
	send_cmd(0x40);
	send_cmd(0xA0);
	send_cmd(0xC0);
	send_cmd(SET_COM_PIN_MAP, 0x02);
	send_cmd(SET_CONTRAST, 0x7F);
	send_cmd(DISPLAY_RAM);
	send_cmd(DISPLAY_NONINV);
	send_cmd(SET_CLK_DIV, 0x80);
	send_cmd(SET_CHARGE_PUMP, 0x14);
	send_cmd(DISPLAY_ON);
	send_cmd(DISPLAY_RAM);

	send_cmd(SET_MEMORY_ADDR_MODE, 0);

	end_cmd_set();

	push_entire_screen();

	xTaskCreate(SSD1306::call_raw_update, "SSD1306 Updater", 2048, this, 3, &updateTask);
	puts("SSD initialized!");
}

XaI2C::MasterAction* SSD1306::start_cmd_set() {
	assert(currentAction == nullptr);
	this->currentAction = new XaI2C::MasterAction(0b0111100);

	return currentAction;
}

void SSD1306::send_cmd(uint8_t cmdByte) {
	assert(currentAction != nullptr);

	cmdBuffer.push_back(cmdByte);
}
void SSD1306::send_cmd(uint8_t cmdByte, uint8_t param) {
	assert(currentAction != nullptr);

	cmdBuffer.push_back(cmdByte);
	cmdBuffer.push_back(param);
}

void SSD1306::end_cmd_set() {
	assert(currentAction != nullptr);

	currentAction->write(0x00, cmdBuffer.data(), cmdBuffer.size());

	currentAction->execute();

	cmdBuffer.clear();
	delete currentAction;
	currentAction = nullptr;
}

void SSD1306::data_write(void *data, size_t length) {
	assert(currentAction == nullptr);

	currentAction = new XaI2C::MasterAction(0b0111100);

	currentAction->write(DATA_STREAM, data, length);
	currentAction->execute();

	delete currentAction;
	currentAction = nullptr;
}

void SSD1306::set_coordinates(uint8_t column, uint8_t page, uint8_t maxColumn, uint8_t maxPage) {
	this->currentAction = start_cmd_set();

	const char oData[] = {SET_COLUMN_RANGE, column, maxColumn, SET_PAGE_RANGE, page, maxPage};
	for(uint8_t i=0; i<6; i++)
		cmdBuffer.push_back(oData[i]);

	end_cmd_set();
}

void SSD1306::clear() {
	for(auto &page : screenBuffer) {
		for(uint8_t i=0; i<page.size(); i++)
			page[i] = 0;
	}
}
void SSD1306::push_entire_screen() {
	set_coordinates();

	for(uint8_t page = 0; page < screenBuffer.size(); page++) {
		data_write(screenBuffer[page].begin(), screenBuffer[page].size());
	}
}

void SSD1306::request_redraw() {
	xTaskNotifyFromISR(updateTask, 0, eNoAction, nullptr);
}

void SSD1306::raw_update() {
	this->clear();
	this->redraw();
	this->push_entire_screen();
}

void SSD1306::set_pixel(int x, int y, int8_t brightness) {
	if(brightness < 0)
		return;
	if(x < 0)
		return;
	if(y < 0)
		return;

	uint8_t page   = y / 8;
	uint8_t page_y = y%8;

	if(page >= screenBuffer.size())
		return;
	if(x >= screenBuffer[page].size())
		return;

	uint8_t &dByte = screenBuffer[page][x];

	if(brightness > 1)
		dByte |= 1<<page_y;
	else
		dByte &= ~(1<<page_y);
}

} /* namespace OLED */
} /* namespace Peripheral */
