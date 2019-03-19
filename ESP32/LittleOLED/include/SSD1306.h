/*
 * SSD1306.h
 *
 *  Created on: 29 Nov 2018
 *      Author: xasin
 */

#ifndef COMPONENTS_LITTLEOLED_SSD1306_H_
#define COMPONENTS_LITTLEOLED_SSD1306_H_

#include <vector>
#include <array>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "MasterAction.h"

#include "DrawBox.h"

#include "fonttype.h"

namespace Peripheral {
namespace OLED {

class SSD1306 : public DrawBox {
public:
	enum CONTROL_BYTE : uint8_t {
		CMD_SINGLE  = 0x80,
		CMD_DOUBLE  = 0x00,
		DATA_STREAM = 0x40
	};

	enum SINGLE_CMD : uint8_t {
		DISPLAY_RAM		= 0xA4,
		DISPLAY_ALLON	= 0xA5,
		DISPLAY_NONINV	= 0xA6,
		DISPLAY_INVERTED= 0xA7,
		DISPLAY_OFF		= 0xAE,
		DISPLAY_ON		= 0xAF,
	};

	enum DOUBLE_CMD : uint8_t {
		SET_CONTRAST 	= 0x81,

		SET_MEMORY_ADDR_MODE = 0x20,
		SET_COLUMN_RANGE	= 0x21,
		SET_PAGE_RANGE		= 0x22,

		SET_MUX_RATIO	= 0xA8,
		SET_DISPLAY_OFFSET = 0xD3,
		SET_COM_PIN_MAP = 0xDA,

		SET_CLK_DIV		= 0xD5,
		SET_PRECHARGE	= 0xD9,
		SET_VCOMH_DESELECT = 0xD8,
		SET_CHARGE_PUMP	= 0x8D,
	};

private:
	static void call_raw_update(void *args);

	XaI2C::MasterAction *currentAction;

	std::vector<char> cmdBuffer;

	std::array<std::array<uint8_t, 128>, 4> screenBuffer;

	TaskHandle_t updateTask;

	XaI2C::MasterAction* start_cmd_set();

	void send_cmd(uint8_t cmdVal);
	void send_cmd(uint8_t cmdVal, uint8_t extraByte);

	void end_cmd_set();

	void data_write(void *data, size_t length);

public:
	SSD1306();

	void initialize();

	void set_coordinates(uint8_t column = 0, uint8_t page = 0, uint8_t maxColumn = 127, uint8_t maxPage = 3);

	void clear();
	void push_entire_screen();

	void request_redraw();
	void raw_update();

	void set_pixel(int x, int y, int8_t brightness = 3);
};

} /* namespace OLED */
} /* namespace Peripheral */

#endif /* COMPONENTS_LITTLEOLED_SSD1306_H_ */
