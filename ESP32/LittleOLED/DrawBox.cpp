/*
 * DrawBox.cpp
 *
 *  Created on: 5 Dec 2018
 *      Author: xasin
 */

#include "DrawBox.h"

namespace Peripheral {
namespace OLED {

#define FONTCHECK if(font == nullptr) {font = &DEFAULT_FONT;}

DrawBox::DrawBox(int width, int height) :
	width(width), height(height),
	topBox(nullptr), bottomBoxes(),
	offsetX(0), offsetY(0),
	rotation(0),
	transparent(false),
	visible(true),
	inverted(false),
	onRedraw(nullptr) {

}
DrawBox::DrawBox(int width, int height, DrawBox *headBox)
	: DrawBox(width, height) {
	set_head(headBox);
}

DrawBox::~DrawBox() {

}

void DrawBox::redraw() {
	for(auto subBox = bottomBoxes.rbegin(); subBox != bottomBoxes.rend(); subBox++)
		(*subBox)->redraw();

	if(onRedraw != nullptr)
		onRedraw();
}

void DrawBox::set_pixel(int x, int y, bool on) {
	if(topBox == nullptr)
		return;
	if(!visible)
		return;
	if(!topBox->visible)
		return;

	if(inverted)
		on = !on;

	if(transparent && !on)
		return;

	if(x < 0 || x > width)
		return;
	if(y < 0 || y > height)
		return;

	int rX = x;
	int rY = y;
	switch(rotation%4) {
	case 1:
		rY = x;
		rX = -y;
	break;
	case 2:
		rX = -x;
		rY = -y;
	break;

	case 3:
		rY = -x;
		rX = y;
	break;
	default: break;
	}

	topBox->set_pixel(rX, rY, on);
}

void DrawBox::draw_line(int x, int y, int l, int r, bool on) {
	short xF = 0;
	short yF = 0;

	switch(r%4) {
	default: xF = 1; break;
	case 1: yF =  1; break;
	case 2: xF = -1; break;
	case 3: yF = -1; break;
	}

	for(int d = 0; d<r; d++)
		set_pixel(x + xF*d, y + yF*d, on);
}
void DrawBox::draw_box(int x, int y, int width, int height, bool filled) {
	width 	-= 1;
	height  -= 1;

	if(filled) {
		for(int dX = 0; dX < width; dX++) {
			draw_line(x+dX, y, height, 1);
		}
	}
	else {
		draw_line(x, y, width, 0);
		draw_line(x, y, height, 1);
		draw_line(x+width, y+height, width, 2);
		draw_line(x+width, y+height, height, 3);
	}
}

int DrawBox::get_line_width(FontType *font) {
	FONTCHECK
	return this->width/(font->width);
}
int DrawBox::get_lines(FontType *font) {
	FONTCHECK
	return this->height/(font->lineHeight);
}

void DrawBox::write_char(int x, int y, char c, bool invert, FontType *font) {
	FONTCHECK

	const char *fontChar = font->fontData + font->height*c;

	if(x >= width)
		return;
	if(y >= height)
		return;
	if((x + font->width) < 0)
		return;
	if((y + font->height) < 0)
		return;

	for(uint8_t dy=0; dy<font->height; dy++) {
		for(uint8_t dx=0; dx<font->width; dx++)
			set_pixel(x+dx, y+dy, ((fontChar[dy]>>(7-dx)) & 1) != invert);
	}
}
void DrawBox::write_string(int x, int y, const std::string oString, bool invert, FontType *font) {
	if(font == nullptr)
		font = &console_font_7x9;

	uint8_t  dL  = 0;
	uint16_t dLx = 0;

	for(uint8_t dc = 0; dc<oString.size(); dc++) {
		char next = oString[dc];
		if(next == '\n') {
			dL++;
			dLx = x + font->width*(1+dc);
		}
		else
			write_char(oString[dc], x + font->width*dc - dLx, y + font->height*dL, invert, font);
	}
}

} /* namespace OLED */
} /* namespace Peripheral */
