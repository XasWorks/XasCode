/*
 * DrawBox.h
 *
 *  Created on: 5 Dec 2018
 *      Author: xasin
 */

#ifndef XASCODE_PATH_LITTLEOLED_DRAWBOX_H_
#define XASCODE_PATH_LITTLEOLED_DRAWBOX_H_

#include <vector>
#include <functional>

#include "fonttype.h"

#ifndef DEFAULT_FONT
#define DEFAULT_FONT console_font_7x9
#endif

namespace Peripheral {
namespace OLED {

class DrawBox {
private:
	int width;
	int height;

	DrawBox *topBox;

protected:
	std::vector<DrawBox *>bottomBoxes;

	virtual void redraw();

public:
	int  offsetX;
	int  offsetY;

	char rotation;

	bool transparent;

	bool visible;
	bool inverted;

	std::function<void (void)> onRedraw;

	DrawBox(int width, int height);
	DrawBox(int width, int height, DrawBox *headBox);
	virtual ~DrawBox();

	void set_head(DrawBox *headBox);

	virtual void request_redraw();

	virtual void set_pixel(int x, int y, bool on = true);

	virtual void draw_line(int x, int y, int l, int r, bool on = true);
	virtual void draw_box(int x, int y, int width, int height, bool filled = true);

	int get_line_width(FontType *font = nullptr);
	int get_lines(FontType *font = nullptr);

	void write_char(int x, int y, char c, bool invert = false, FontType *font = nullptr);
	void write_string(int x, int y, const std::string oString, bool invert = false, FontType *font = nullptr);
};

} /* namespace OLED */
} /* namespace Peripheral */

#endif /* XASCODE_PATH_LITTLEOLED_DRAWBOX_H_ */
