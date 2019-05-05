/*
 * SSD1327.h
 *
 *  Created on: 29 Nov 2018
 *      Author: xasin
 */

#ifndef COMPONENTS_LITTLEOLED_SSD1327_H_
#define COMPONENTS_LITTLEOLED_SSD1327_H_

#include <vector>
#include <array>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "MasterAction.h"

#include "DrawBox.h"

#include "fonttype.h"

namespace Peripheral {
namespace OLED {

class SSD1327 : public DrawBox {
public:
	enum CONTROL_BYTE : uint8_t {
		CMD_SINGLE  = 0x80,
		CMD_DOUBLE  = 0x00,
		DATA_STREAM = 0x40
	};

	enum SINGLE_CMD : uint8_t {
		DISPLAY_RAM		= 0xA4,
		DISPLAY_ALLON	= 0xA5,
		DISPLAY_ALLOFF	= 0xA6,
		DISPLAY_INVERTED= 0xA7,

		DISPLAY_DISABLE	= 0xAE,
		DISPLAY_ENABLE	= 0xAF,

		ENABLE_LIN_GREYSCALE = 0xB9,
	};

	enum DOUBLE_CMD : uint8_t {
		SET_CONTRAST 		 = 0x81,

		SET_COLUMN_RANGE	= 0x15,
		SET_ROW_RANGE		= 0x75,

		SET_MUX_RATIO		= 0xA8,
		SET_REMAP			= 0xA0,
		SELECT_VDD_SRC		= 0xAB,
	};

private:
	static void call_raw_update(void *args);

	XaI2C::MasterAction *currentAction;

	std::vector<char> cmdBuffer;

	std::array<uint8_t, 254>	greyscaleBuffer;
	std::array<std::array<uint16_t, 128>, 16> screenBuffer;

	std::vector<DirtyArea> redrawAreas;

	TaskHandle_t updateTask;

	XaI2C::MasterAction* start_cmd_set();

	void send_cmd(uint8_t cmdVal);
	void send_cmd(uint8_t cmdVal, uint8_t extraByte);

	void end_cmd_set();

	void data_write(void *data, size_t length);

public:
	SSD1327();

	void initialize();

	void set_coordinates(uint8_t column = 0, uint8_t page = 0, uint8_t maxColumn = 64, uint8_t maxPage = 127);

	void clear();
	void push_dirty_areas();
	void push_segment(uint8_t lColumn = 0, uint8_t tRow = 0, uint8_t rColumn = 64, uint8_t bRow = 128);
	void push_entire_screen();

	void request_redraw(bool force = false);
	void raw_update();

	void mark_dirty_area(DirtyArea area, bool force = false);
	void set_pixel(int x, int y, int8_t brightness = 3);
};

} /* namespace OLED */
} /* namespace Peripheral */

#endif /* COMPONENTS_LITTLEOLED_SSD1327_H_ */
