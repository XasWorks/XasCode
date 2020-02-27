/*
 * Color.cpp
 *
 *  Created on: 19 Sep 2018
 *      Author: xasin
 */


#include <NeoController/Color.h>
#include <cmath>

namespace Xasin {

Color Color::HSV(float H, float S, float V) {
	H = fmodf(H, 360);
	if(H < 0)
		H += 360;

	uint8_t h = floorf(H)/60;

	float f = (H - 60*h)/60.0f;

	float p = V * (1 - S);

	float q = V * (1 - S*f);
	float t = V * (1 - S*(1 - f));

	Color oC = Color();

	switch(h) {
	default:oC.r = V; oC.g = t; oC.b = p; break;
	case 1: oC.r = q; oC.g = V; oC.b = p; break;
	case 2: oC.r = p; oC.g = V; oC.b = t; break;
	case 3: oC.r = p; oC.g = q; oC.b = V; break;
	case 4: oC.r = t; oC.g = p; oC.b = V; break;
	case 5: oC.r = V; oC.g = p; oC.b = q; break;
	}

	return oC;
}

Color::Color() {
	r = 0;
	g = 0;
	b = 0;

	alpha = 1;
}
Color::Color(uint32_t cCode, float brightness) : Color() {
	set(cCode);
	bMod(brightness);
}
Color::Color(uint32_t cCode, float brightness, float alpha) : Color(cCode, brightness) {
	this->alpha = alpha;
}

uint8_t Color::f_to_u8(float f) const {
	if(f > 1.0f)
		return 255;
	if(f < 0.0f)
		return 0;

	return roundf(f*f*(255));
}

Color::ColorData Color::getLEDValue() const {
	ColorData out = {};

	out.r = f_to_u8(r);
	out.g = f_to_u8(g);
	out.b = f_to_u8(b);

	return out;
}

uint32_t Color::getPrintable() const {
	auto data = this->getLEDValue();

	return (data.r*255) << 16 | (data.g*255) << 8 | (data.b*255);
}

void Color::set(const Color &color) {
	r = color.r;
	g = color.g;
	b = color.b;

	alpha = color.alpha;
}
void Color::set(uint32_t cCode) {
	uint8_t *colorPart = (uint8_t *)&cCode;

	r = colorPart[2]/255.0f;
	g = colorPart[1]/255.0f;
	b = colorPart[0]/255.0f;
}
void Color::set(uint32_t cCode, float factor) {
	set(cCode);
	bMod(factor);
}
Color& Color::operator=(const Color& nColor) {
	this->set(nColor);

	return *this;
}

Color &Color::bMod(float factor) {
	r = r*factor;
	g = g*factor;
	b = b*factor;

	return *this;
}
Color Color::bMod(float factor) const {
	Color oColor = *this;

	return oColor.bMod(factor);
}

Color Color::overlay(Color topColor, float alpha) {
	/*Color oColor = Color();
	for(uint8_t i=0; i<3; i++)
		*(&(oColor.r) + i) = ((uint16_t)*(&this->r + i)*(255 - alpha) + *(&topColor.r +i)*(alpha)) >> 8;
	*/

	return (*this)*(1-alpha) + topColor*alpha;
}
void Color::overlay(Color bottom, Color top, float alpha) {
	for(uint8_t i=0; i<3; i++)
		(&this->r)[i] = (uint32_t((&bottom.r)[i])*(1 - alpha) + (&top.r)[i]*(alpha));
}

Color Color::operator +(Color secondColor) {
	Color oColor = *this;
	for(uint8_t i=0; i<3; i++)
		(&(oColor.r))[i] = (&secondColor.r)[i] + (&this->r)[i];

	return oColor;
}
Color Color::operator *(float brightness) {
	Color oColor = *this;
	oColor.bMod(brightness);

	return oColor;
}

#define MERGE_OVERLAY(code) (this->code) = (this->code * own_transmission_p) + (top.code * (1 - own_transmission_p))
Color& Color::merge_overlay(const Color &top, float alpha) {
	float total_alpha_top = top.alpha * alpha;

	float own_transmission = this->alpha * (1.0f - total_alpha_top);
	float own_transmission_p = 0;

	if(own_transmission != 0)
		own_transmission_p = own_transmission / (own_transmission + total_alpha_top);

	MERGE_OVERLAY(r);
	MERGE_OVERLAY(g);
	MERGE_OVERLAY(b);

	this->alpha = (1 - (1 - this->alpha) * (1 - total_alpha_top));

	return *this;
}

#define MERGE_MULT_COLOR(code) this->code = this->code * (1 - total_alpha + total_alpha * top.code);

Color& Color::merge_multiply(const Color &top, float alpha) {
	uint32_t total_alpha = top.alpha * alpha;

	MERGE_MULT_COLOR(r);
	MERGE_MULT_COLOR(g);
	MERGE_MULT_COLOR(b);

	return *this;
}
#define MERGE_MULT_SCALAR(code) (code) = (code*scalar);
Color& Color::merge_multiply(float scalar) {
	MERGE_MULT_SCALAR(r);
	MERGE_MULT_SCALAR(g);
	MERGE_MULT_SCALAR(b);

	return *this;
}
Color& Color::merge_add(const Color &top, float alpha) {
	uint16_t total_alpha = top.alpha * alpha;

	for(uint8_t i=0; i<3; i++) {
		float& c = (&r)[i];
		c = fminf(1, fmaxf(0, c + (&top.r)[i] * total_alpha));
	}
	this->alpha = fminf(1, this->alpha + fabsf(total_alpha));

	return *this;
}

#define T_MERGE(code) (this->code) = (top.code * alpha) + (this->code * (1-alpha))
Color& Color::merge_transition(const Color &top, float alpha) {
	T_MERGE(r);
	T_MERGE(g);
	T_MERGE(b);
	T_MERGE(alpha);

	return *this;
}

Color Color::calculate_overlay(const Color &top, float alpha) const {
	Color oColor = *this;

	return (oColor.merge_overlay(top, alpha));
}
Color Color::calculate_multiply(const Color &top, float alpha) const {
	Color oColor = *this;

	return (oColor.merge_multiply(top, alpha));
}
Color Color::calculate_multiply(float scalar) const {
	Color oColor = *this;

	return (oColor.merge_multiply(scalar));
}
Color Color::calculate_add(const Color &top, float alpha) const {
	Color oColor = *this;

	return (oColor.merge_add(top, alpha));
}

} /* namespace Peripheral */
