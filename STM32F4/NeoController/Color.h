/*
 * Color.h
 *
 *  Created on: 19 Sep 2018
 *      Author: xasin
 */

#ifndef COMPONENTS_NEOCONTROLLER_COLOR_H_
#define COMPONENTS_NEOCONTROLLER_COLOR_H_

#include <stdint.h>

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

namespace Xasin {

class Color {
private:
	uint8_t f_to_u8(float f) const;

public:
	struct ColorData {
		union {
			struct {
				uint8_t b;
				uint8_t r;
				uint8_t g;
			};
			uint32_t data;
		};
	};

	float r;
	float g;
	float b;
	float alpha;

	static Color HSV(float H, float S = 1, float V = 1);

	Color();
	Color(uint32_t cCode, float brightness = 1);
	Color(uint32_t cCode, float brightness, float alpha);

	ColorData getLEDValue() const;
	uint32_t  getPrintable() const;

	void set(uint32_t cCode);
	void set(uint32_t cCode, float bMod);
	void set(const Color &color);

	Color& operator=(const Color& nColor);

	Color& bMod(float div);
	Color  bMod(float div) const;

	Color overlay(Color topColor, float alpha);
	void  overlay(Color bottom, Color top, float alpha);

	Color operator +(Color secondColor);
	Color operator *(float brightness);

	Color& merge_overlay(const Color &top, float alpha = 1);
	Color& merge_multiply(const Color &top, float alpha = 1);
	Color& merge_multiply(float scalar);
	Color& merge_add(const Color &top, float alpha = 1);

	Color& merge_transition(const Color &top, float alpha);

	Color calculate_overlay(const Color &top, float alpha = 1) const;
	Color calculate_multiply(const Color &top, float alpha = 1) const;
	Color calculate_multiply(float scalar) const;
	Color calculate_add(const Color &top, float alpha = 1) const;
};


} /* namespace Peripheral */

#endif /* COMPONENTS_NEOCONTROLLER_COLOR_H_ */
