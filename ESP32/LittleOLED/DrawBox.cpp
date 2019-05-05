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
	topBox(nullptr), bottomBoxes(),
	width(width), height(height),
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
	set_visible(false);
	set_head(nullptr);
}

void DrawBox::set_head(DrawBox *head, bool registerCB) {
	if(this->topBox != nullptr)
		for(auto box = topBox->bottomBoxes.begin(); box < topBox->bottomBoxes.end(); box++)
			if((*box) == this)
				topBox->bottomBoxes.erase(box);

	if(registerCB && (head != nullptr))
		head->bottomBoxes.push_back(this);

	this->topBox = head;
}

Point DrawBox::remap_point(Point in) {
	int rX = in.x;
	int rY = in.y;
	switch(rotation%4) {
	case 1:
		rY = -in.x;
		rX = in.y;
	break;
	case 2:
		rX = -in.x;
		rY = -in.y;
	break;

	case 3:
		rY = in.x;
		rX = -in.y;
	break;
	default: break;
	}

	return {rX + offsetX, rY + offsetY};
}

void DrawBox::mark_dirty_area(DirtyArea area, bool force) {
	if((visible || force) && (topBox != nullptr)) {
		Point remapped = remap_point({area.startX, area.startY});
		DirtyArea outArea = {remapped.x, 0, remapped.y, 0};

		remapped = remap_point({area.endX, area.endY});
		outArea.endX = remapped.x;
		outArea.endY = remapped.y;

		topBox->mark_dirty_area(outArea);
	}
}
void DrawBox::redraw() {
	if(!visible)
		return;

	for(auto subBox = bottomBoxes.rbegin(); subBox != bottomBoxes.rend(); subBox++)
		(*subBox)->redraw();

	if(onRedraw != nullptr)
		onRedraw();
}
void DrawBox::request_redraw(bool force) {
	if((topBox != nullptr) && (visible || force))
		topBox->request_redraw();
}

void DrawBox::set_pixel(int x, int y, int8_t brightness) {
	if(brightness < 0)
		return;
	if(topBox == nullptr)
		return;
	if(!visible)
		return;
	if(!topBox->visible)
		return;

	if(inverted)
		brightness = 3 - brightness;

	if(transparent && (brightness == 0))
		return;

	if((x < 0) || (x >= width))
		return;
	if((y < 0) || (y >= height))
		return;

	Point remapped = remap_point({x, y});

	topBox->set_pixel(remapped.x, remapped.y, brightness);
}

void DrawBox::draw_line(int x, int y, int l, int r, int8_t brightness) {
	if(brightness < 0)
		return;

	short xF = 0;
	short yF = 0;

	switch(r%4) {
	default: xF = 1; break;
	case 1: yF =  1; break;
	case 2: xF = -1; break;
	case 3: yF = -1; break;
	}

	for(int d = 0; d<l; d++)
		set_pixel(x + xF*d, y + yF*d, brightness);
}
void DrawBox::draw_box(int x, int y, int width, int height, int8_t oBrightness, int8_t iBrightness) {
	if(iBrightness >= 0) {
		for(int dX = 0; dX < width; dX++) {
			draw_line(x+dX, y, height, 1, iBrightness);
		}
	}

	draw_line(x, y, width, 0, oBrightness);
	draw_line(x, y, height, 1, oBrightness);
	draw_line(x+width-1, y+height-1, width, 2, oBrightness);
	draw_line(x+width-1, y+height-1, height, 3, oBrightness);

}

int DrawBox::get_line_width(FontType *font) {
	FONTCHECK
	return this->width/(font->width);
}
int DrawBox::get_lines(FontType *font) {
	FONTCHECK
	return this->height/(font->lineHeight);
}

void DrawBox::write_char(int x, int y, char c, int8_t foreground, int8_t background, FontType *font) {
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
			set_pixel(x+dx, y+dy, ((fontChar[dy]>>(7-dx)) & 1) ? foreground : background);
	}
}
void DrawBox::write_string(int x, int y, const std::string oString, int8_t foreground, int8_t background, FontType *font) {
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
			write_char(x + font->width*dc - dLx, y + font->height*dL, oString[dc], foreground, background, font);
	}
}

void DrawBox::set_invert(bool inverted) {
	if(inverted == this->inverted)
		return;

	this->inverted = inverted;
	mark_dirty_area({0, width-1, 0, height-1});

	request_redraw();
}

void DrawBox::set_visible(bool visible) {
	if(visible == this->visible)
		return;

	mark_dirty_area({0, width-1, 0, height-1}, true);
	this->visible = visible;

	request_redraw(true);
}

} /* namespace OLED */
} /* namespace Peripheral */
