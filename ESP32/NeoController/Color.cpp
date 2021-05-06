/*
 * Color.cpp
 *
 *  Created on: 19 Sep 2018
 *      Author: xasin
 */

#include "xasin/neocontroller/Color.h"

#include <algorithm>
#include <math.h>

#include "esp_log.h"

#define RAW_C_MAX (255*257)
#define BOUND_RAW_C(v) std::min<int32_t>(RAW_C_MAX, std::max<int32_t>(0, v))
#define U8_TO_RAW_C(v) uint8_t(v) * uint16_t(257)
#define RAW_TO_U8(v)   uint8_t(v/257)


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
	default:oC.r = V; oC.g = t; oC.b = p; break;
	case 1: oC.r = q; oC.g = V*255; oC.b = p; break;
	case 2: oC.r = p; oC.g = V*255; oC.b = t; break;
	case 3: oC.r = p; oC.g = q; oC.b = V*255; break;
	case 4: oC.r = t; oC.g = p; oC.b = V*255; break;
	case 5: oC.r = V*255; oC.g = p; oC.b = q; break;
	}

	return oC;
}

#define MIN_B (0.2F)
#define MIN_B_TEMP (9)
#define MAX_B_TEMP (20)
Color Color::Temperature(float temperature, float brightness) {
	Xasin::NeoController::Color out = 0;

    float r_temp = 0;
    float g_temp = 0;
    float b_temp = 0;

    temperature /= 100;
    if(temperature <= 66) {
        r_temp = 255;

        g_temp = temperature - 2;
        g_temp = -155.25485562709179F - 0.44596950469579133F * g_temp + 104.49216199393888F * logf(g_temp); // 99.4708025861F * logf(g_temp) - 161.1195681661F;

        if(temperature > 19) {
            b_temp = temperature - 10;
			b_temp = -254.76935184120902F + 0.8274096064007395F * b_temp + 115.67994401066147F * logf(b_temp);
			//b_temp = 138.5177312231F * logf(b_temp) - 305.0447927307F;
        }
    }
    else {
        r_temp = temperature - 55;
        r_temp = 351.97690566805693F + 0.114206453784165 * r_temp - 40.25366309332127F * logf(r_temp);

        g_temp = temperature - 50;
        g_temp = 325.4494125711974F + 0.07943456536662342F * g_temp - 28.0852963507957F * logf(g_temp);
		//g_temp = 288.1221695283F * powf(g_temp, -0.0755148492F);
    
        b_temp = 255;
    }

    if(r_temp >= 255)
        out.r = RAW_C_MAX;
    else if(r_temp > 0)
        out.r = U8_TO_RAW_C(r_temp);

    if(g_temp >= 255)
        out.g = RAW_C_MAX;
    else if((g_temp > 0) && (temperature <= 66))
        out.g = U8_TO_RAW_C(255*sqrtf(g_temp/255));
	else if(g_temp > 0)
		out.g = U8_TO_RAW_C(g_temp);

    if(b_temp >= 255) {
        out.b = RAW_C_MAX;
	}
	else if(b_temp > 0) {
        out.b = U8_TO_RAW_C(b_temp);
	}

	if(brightness <= -1) {
		if(temperature < MIN_B_TEMP)
			brightness = MIN_B;
		else if(temperature < MAX_B_TEMP)
			brightness = MIN_B + (1 - MIN_B) * logf(temperature / MIN_B_TEMP) / logf(MAX_B_TEMP/MIN_B_TEMP);
		else
			brightness = 1;
	}

    out.bMod(255 * std::min(1.0F, std::max(0.0F, brightness)));

	return out;
}

Color Color::strtoc(const char *str) {
	if(str == nullptr)
		return 0;
	if(*str == '#') str++;

	if(*str == 0)
		return 0;

	return Color(strtol(str, nullptr, 16));
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
	out.r = (uint32_t(r)*r)/(RAW_C_MAX) >> 8;
	out.g = (uint32_t(g)*g)/(RAW_C_MAX) >> 8;
	out.b = (uint32_t(b)*b)/(RAW_C_MAX) >> 8;

	return out;
}

uint32_t Color::getPrintable() const {
	return (uint32_t(RAW_TO_U8(r)) << 16) | (uint32_t(RAW_TO_U8(g)) << 8) | (RAW_TO_U8(b));
}

void Color::set(Color color) {
	for(uint8_t i=0; i<4; i++) {
		*(&this->r + i) = *(&color.r + i);
	}
}
void Color::set(uint32_t cCode) {
	uint8_t *colorPart = (uint8_t *)&cCode;
	r = U8_TO_RAW_C(colorPart[2]);
	g = U8_TO_RAW_C(colorPart[1]);
	b = U8_TO_RAW_C(colorPart[0]);
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

Color Color::operator +(Color secondColor) const {
	Color oColor = *this;
	for(uint8_t i=0; i<3; i++)
		(&(oColor.r))[i] = (&secondColor.r)[i] + (&this->r)[i];

	return oColor;
}
Color Color::operator *(uint8_t brightness) const {
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

		c = BOUND_RAW_C(cB);
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
