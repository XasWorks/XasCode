/*
 * Color.cpp
 *
 *  Created on: 19 Sep 2018
 *      Author: xasin
 */

#include "xasin/neocontroller/Color.h"

namespace Xasin {
namespace NeoController {

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

	alpha = 255;
}
Color::Color(uint32_t cCode, uint8_t brightness) : Color() {
	set(cCode);
	bMod(brightness);
}
Color::Color(uint32_t cCode, uint8_t brightness, uint8_t alpha) : Color(cCode, brightness) {
	this->alpha = alpha;
}

Color::ColorData Color::getLEDValue() const {
	ColorData out = {};
	out.r = ((uint32_t)r*r)/0xFFFFFF;
	out.g = ((uint32_t)g*g)/0xFFFFFF;
	out.b = ((uint32_t)b*b)/0xFFFFFF;

	return out;
}

uint32_t Color::getPrintable() const {
	auto data = this->getLEDValue();

	return data.r << 16 | data.g << 8 | data.b;
}

void Color::set(Color color) {
	for(uint8_t i=0; i<4; i++) {
		*(&this->r + i) = *(&color.r + i);
	}
}
void Color::set(uint32_t cCode) {
	uint8_t *colorPart = (uint8_t *)&cCode;
	r = uint16_t(colorPart[2])*257;
	g = uint16_t(colorPart[1])*257;
	b = uint16_t(colorPart[0])*257;
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

#define MERGE_OVERLAY(code) (this->code) = (uint32_t(this->code)*own_transmission_p + uint32_t(top.code)*(65025-own_transmission_p))/(65025)
Color& Color::merge_overlay(const Color &top, uint8_t alpha) {
	uint16_t total_alpha_top = (top.alpha * uint16_t(alpha)) / 255;

	uint32_t own_transmission = this->alpha * (255 - total_alpha_top);
	uint32_t own_transmission_p = 0;
	if(own_transmission != 0)
		own_transmission_p = (65025 * own_transmission) / (own_transmission + 255*total_alpha_top);

	MERGE_OVERLAY(r);
	MERGE_OVERLAY(g);
	MERGE_OVERLAY(b);

	this->alpha = (65025 - (255 - this->alpha)*(255 - total_alpha_top)) / 255;

	return *this;
}
#define MERGE_MULT_COLOR(code) this->code = (this->code * (65025-total_alpha + (total_alpha*((top.code))/65025)))/65025;

Color& Color::merge_multiply(const Color &top, uint8_t alpha) {
	uint32_t total_alpha = (top.alpha * uint16_t(alpha));

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
	uint8_t total_alpha = (top.alpha * uint16_t(alpha))/255;

	for(uint8_t i=0; i<3; i++) {
		uint16_t& c = (&r)[i];
		uint32_t cB = c + ((&top.r)[i]*total_alpha/255);
		if(cB > 65535)
			cB = 65535;

		c = cB;
	}
	uint16_t alphaB = this->alpha + total_alpha;
	if(alphaB > 255)
		alphaB = 255;
	this->alpha = alphaB;

	return *this;
}

#define T_MERGE(code) (this->code) = (uint32_t(top.code)*alpha + uint32_t(this->code)*(65025-alpha))/(65025);
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

}
} /* namespace Peripheral */
