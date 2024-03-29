/*
 * Color.h
 *
 *  Created on: 19 Sep 2018
 *      Author: xasin
 */

#ifndef COMPONENTS_NEOCONTROLLER_COLOR_H_
#define COMPONENTS_NEOCONTROLLER_COLOR_H_

#include "driver/rmt.h"
#include <stdint.h>

#include <string>

enum Material : uint32_t {
	BLACK 	= 0x000000,
	RED 	= 0xF42316,
	PINK	= 0xE91E63,
	PURPLE	= 0x9C27B0,
	DEEP_PURPLE = 0x6A3AB7,
	INDIGO	= 0x3F51B5,
	BLUE	= 0x0546FF,
	CYAN	= 0x00CCE4,
	GREEN	= 0x20DF2C,
	LIME	= 0xCCE210,
	YELLOW	= 0xEBEB00,
	AMBER	= 0xFFC007,
	ORANGE	= 0xFF9800,
	DEEP_ORANGE = 0xFF4700
};

namespace XNM {
namespace Neo {

class Color {
public:
#pragma pack(1)
	struct ColorData {
		uint8_t g;
		uint8_t r;
		uint8_t b;
	};
#pragma pack(0)

	uint16_t r;
	uint16_t g;
	uint16_t b;
	uint16_t alpha;

	static Color HSV(int16_t H, uint8_t S = 255, uint8_t V = 255);
	static Color Temperature(float temp, float brightness = -1);
	static Color strtoc(const char *str, bool *ok = nullptr);

	Color();
	Color(uint32_t cCode, uint8_t brightness = 255);
	Color(uint32_t cCode, uint8_t brightness, uint8_t alpha);

	ColorData getLEDValue() const;
	uint32_t  getPrintable() const;

	std::string to_s() const;

	void set(uint32_t cCode);
	void set(uint32_t cCode, uint8_t div);
	void set(Color color);
	Color& operator=(const Color& nColor);

	bool operator==(const Color& compare) const;

	Color& bMod(uint8_t div);
	Color  bMod(uint8_t div) const;

	Color overlay(Color topColor, uint8_t level);
	void  overlay(Color bottom, Color top, uint8_t alpha);

	Color operator +(Color secondColor) const;
	Color operator *(uint8_t brightness) const;

	Color& merge_overlay(const Color &top, uint8_t alpha = 255);
	Color& merge_multiply(const Color &top, uint8_t alpha = 255);
	Color& merge_multiply(uint8_t scalar);
	Color& merge_add(const Color &top, uint8_t alpha = 255);

	Color& merge_transition(const Color &top, uint16_t alpha);

	Color calculate_overlay(const Color &top, uint8_t alpha = 255) const;
	Color calculate_multiply(const Color &top, uint8_t alpha = 255) const;
	Color calculate_multiply(uint8_t scalar) const;
	Color calculate_add(const Color &top, uint8_t alpha = 255) const;
};

}
} /* namespace Peripheral */

#endif /* COMPONENTS_NEOCONTROLLER_COLOR_H_ */
