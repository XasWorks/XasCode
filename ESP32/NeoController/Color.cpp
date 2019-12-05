/*
 * Color.cpp
 *
 *  Created on: 19 Sep 2018
 *      Author: xasin
 */

#include "Color.h"

#define MAX_COLOR_VAL (65535)
#define MULT_AND_DOWNSCALE(a, b) uint16_t((uint32_t(a) * uint32_t(b)) / MAX_COLOR_VAL)
#define B16_TO_B8(a) uint8_t(a / 257)
#define B8_TO_B16(a) (uint16_t(a) * 257)

namespace Peripheral {

Color Color::HSV(int16_t H, uint8_t S, uint8_t V) {
	H %= 360;
	if(H < 0)
		H += 360;

	uint16_t h = (H/60);
	uint16_t f = ((uint32_t(H)*255)/60) % 255;

	uint16_t p = uint16_t(V) * (255 - S);
	uint16_t q = uint16_t(V) * (255 - (S*f)/255);
	uint16_t t = uint16_t(V) * (255 - (S*(255 - f))/255);

	Color oC = Color();

	switch(h) {
	default:oC.r = V*255; oC.g = t; oC.b = p; break;
	case 1: oC.r = q; oC.g = V*255; oC.b = p; break;
	case 2: oC.r = p; oC.g = V*255; oC.b = t; break;
	case 3: oC.r = p; oC.g = q; oC.b = V*255; break;
	case 4: oC.r = t; oC.g = p; oC.b = V*255; break;
	case 5: oC.r = V*255; oC.g = p; oC.b = q; break;
	}

	return oC;
}

Color::Color() {
	r = 0;
	g = 0;
	b = 0;

	alpha = MAX_COLOR_VAL;
}
Color::Color(uint32_t cCode, uint8_t brightness) : Color() {
	set(cCode);
	bMod(brightness);
}
Color::Color(uint32_t cCode, uint8_t brightness, uint8_t alpha) : Color(cCode, brightness) {
	this->alpha = B8_TO_B16(alpha);
}

Color::ColorData Color::getLEDValue() const {
	ColorData out = {};
	out.r = B16_TO_B8(MULT_AND_DOWNSCALE(r, r));
	out.g = B16_TO_B8(MULT_AND_DOWNSCALE(r, r));
	out.b = B16_TO_B8(MULT_AND_DOWNSCALE(r, r));

	return out;
}

uint32_t Color::getPrintable() const {
	auto data = this->getLEDValue();

	return (data.r) << 16 | (data.g/255) << 8 | (data.b/255);
}

void Color::set(Color color) {
	r = color.r;
	g = color.g;
	b = color.g;

	alpha = color.alpha;
}
void Color::set(uint32_t cCode) {
	uint8_t *colorPart = (uint8_t *)&cCode;

	r = B8_TO_B16(colorPart[2]);
	g = B8_TO_B16(colorPart[1]);
	b = B8_TO_B16(colorPart[0]);
}
void Color::set(uint32_t cCode, uint8_t factor) {
	set(cCode);
	bMod(factor);
}
Color& Color::operator=(const Color& nColor) {
	this->set(nColor);

	return *this;
}

Color &Color::bMod(uint8_t factor) {
	uint32_t bFact = factor;

	r = ((r*bFact)/255);
	g = ((g*bFact)/255);
	b = ((b*bFact)/255);

	return *this;
}
Color Color::bMod(uint8_t factor) const {
	Color oColor = *this;

	return oColor.bMod(factor);
}

Color Color::overlay(Color topColor, uint8_t alpha) {
	/*Color oColor = Color();
	for(uint8_t i=0; i<3; i++)
		*(&(oColor.r) + i) = ((uint16_t)*(&this->r + i)*(255 - alpha) + *(&topColor.r +i)*(alpha)) >> 8;
	*/

	return (*this)*(255-alpha) + topColor*(alpha);
}
void Color::overlay(Color bottom, Color top, uint8_t alpha) {
	for(uint8_t i=0; i<3; i++)
		(&this->r)[i] = (uint32_t((&bottom.r)[i])*(255 - alpha) + (&top.r)[i]*(alpha))/255;
}

Color Color::operator +(Color secondColor) {
	Color oColor = *this;
	for(uint8_t i=0; i<3; i++)
		(&(oColor.r))[i] = (&secondColor.r)[i] + (&this->r)[i];

	return oColor;
}
Color Color::operator *(uint8_t brightness) {
	Color oColor = *this;
	oColor.bMod(brightness);

	return oColor;
}

#define MERGE_OVERLAY(code) (this->code) = (MULT_AND_DOWNSCALE(this->code, own_transmission_p) + MULT_AND_DOWNSCALE(top.code,(MAX_COLOR_VAL-own_transmission_p)))
Color& Color::merge_overlay(const Color &top, uint8_t alpha) {
	uint16_t total_alpha_top = (top.alpha * uint32_t(alpha)) / 255;

	uint32_t own_transmission = MULT_AND_DOWNSCALE(this->alpha, (MAX_COLOR_VAL - total_alpha_top));
	uint32_t own_transmission_p = 0;

	if(own_transmission != 0)
		own_transmission_p = (MAX_COLOR_VAL * own_transmission) / (own_transmission + total_alpha_top);

	MERGE_OVERLAY(r);
	MERGE_OVERLAY(g);
	MERGE_OVERLAY(b);

	this->alpha = (MAX_COLOR_VAL - MULT_AND_DOWNSCALE((MAX_COLOR_VAL - this->alpha), (MAX_COLOR_VAL - total_alpha_top)));

	return *this;
}

#define MERGE_MULT_COLOR(code) this->code = MULT_AND_DOWNSCALE(this->code,  (MAX_COLOR_VAL-total_alpha + MULT_AND_DOWNSCALE(total_alpha, top.code)));

Color& Color::merge_multiply(const Color &top, uint8_t alpha) {
	uint32_t total_alpha = MULT_AND_DOWNSCALE(top.alpha, B8_TO_B16(alpha));

	MERGE_MULT_COLOR(r);
	MERGE_MULT_COLOR(g);
	MERGE_MULT_COLOR(b);

	return *this;
}
#define MERGE_MULT_SCALAR(code) (code) = ((code)*uint32_t(scalar)) / 255;
Color& Color::merge_multiply(uint8_t scalar) {
	MERGE_MULT_SCALAR(r);
	MERGE_MULT_SCALAR(g);
	MERGE_MULT_SCALAR(b);

	return *this;
}
Color& Color::merge_add(const Color &top, uint8_t alpha) {
	uint16_t total_alpha = MULT_AND_DOWNSCALE(top.alpha, B8_TO_B16(alpha));

	for(uint8_t i=0; i<3; i++) {
		uint16_t& c = (&r)[i];
		uint32_t cB = c + MULT_AND_DOWNSCALE((&top.r)[i], total_alpha);
		if(cB > MAX_COLOR_VAL)
			cB = MAX_COLOR_VAL;

		c = cB;
	}
	uint32_t alphaB = this->alpha + total_alpha;
	if(alphaB > MAX_COLOR_VAL)
		alphaB = MAX_COLOR_VAL;

	this->alpha = alphaB;

	return *this;
}

#define T_MERGE(code) (this->code) = (MULT_AND_DOWNSCALE(top.code, alpha) + MULT_AND_DOWNSCALE(this->code, MAX_COLOR_VAL-alpha))
Color& Color::merge_transition(const Color &top, uint16_t alpha) {
	T_MERGE(r);
	T_MERGE(g);
	T_MERGE(b);
	T_MERGE(alpha);

	return *this;
}

Color Color::calculate_overlay(const Color &top, uint8_t alpha) const {
	Color oColor = *this;

	return (oColor.merge_overlay(top, alpha));
}
Color Color::calculate_multiply(const Color &top, uint8_t alpha) const {
	Color oColor = *this;

	return (oColor.merge_multiply(top, alpha));
}
Color Color::calculate_multiply(uint8_t scalar) const {
	Color oColor = *this;

	return (oColor.merge_multiply(scalar));
}
Color Color::calculate_add(const Color &top, uint8_t alpha) const {
	Color oColor = *this;

	return (oColor.merge_add(top, alpha));
}

} /* namespace Peripheral */
