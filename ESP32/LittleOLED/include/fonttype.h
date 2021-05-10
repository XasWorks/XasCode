/*
 * FontType.h
 *
 *  Created on: 5 Dec 2018
 *      Author: xasin
 */

#ifndef XASCODE_PATH_LITTLEOLED_FONTS_FONTTYPE_H_
#define XASCODE_PATH_LITTLEOLED_FONTS_FONTTYPE_H_

namespace Peripheral {
namespace OLED {

struct FontType {
	const char width;
	const char height;
	const char lineHeight;

	const char *fontData;
};

extern FontType console_font_5x8;
extern FontType console_font_6x8;
extern FontType console_font_7x9;

extern const char raw_console_font_5x8[];
extern const char raw_console_font_6x8[];
extern const char raw_console_font_7x9[];

}
}

#endif /* XASCODE_PATH_LITTLEOLED_FONTS_FONTTYPE_H_ */
