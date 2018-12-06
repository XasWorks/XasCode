/*
 * DrawBox.cpp
 *
 *  Created on: 5 Dec 2018
 *      Author: xasin
 */

#include "DrawBox.h"

#include <stdio.h>

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

void DrawBox::set_head(DrawBox *head) {
	if(this->topBox != nullptr)
		for(auto box = topBox->bottomBoxes.begin(); box < topBox->bottomBoxes.end(); box++)
			if((*box) == this)
				topBox->bottomBoxes.erase(box);

	if(head != nullptr)
		head->bottomBoxes.push_back(this);

	this->topBox = head;
}

void DrawBox::redraw() {
	for(auto subBox = bottomBoxes.rbegin(); subBox != bottomBoxes.rend(); subBox++)
		(*subBox)->redraw();

	if(onRedraw != nullptr)
		onRedraw();
}
void DrawBox::request_redraw() {
	if(topBox != nullptr)
		topBox->request_redraw();
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
		rY = -x;
		rX = y;
	break;
	case 2:
		rX = -x;
		rY = -y;
	break;

	case 3:
		rY = x;
		rX = -y;
	break;
	default: break;
	}

	topBox->set_pixel(rX + offsetX, rY + offsetY, on);
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
	FONTCHECK

	uint8_t  dL  = 0;
	uint16_t dLx = 0;

	for(uint8_t dc = 0; dc<oString.size(); dc++) {
		char next = oString[dc];
		if(next == '\n') {
			dL++;
			dLx = x + font->width*(1+dc);
		}
		else
			write_char(x + font->width*dc - dLx, y + font->height*dL, oString[dc], invert, font);
	}
}

} /* namespace OLED */
} /* namespace Peripheral */
