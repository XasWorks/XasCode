/*
 * Color.h
 *
 *  Created on: 19 Sep 2018
 *      Author: xasin
 */

#ifndef COMPONENTS_NEOCONTROLLER_LAYER_H_
#define COMPONENTS_NEOCONTROLLER_LAYER_H_

#include <vector>

#include "Color.h"

namespace Peripheral {
	class Layer {
	public:
		std::vector<Color> colors;

		uint8_t alpha;

		Layer(const int length);
		Layer(const Layer &source);

		int length() const;

		Color& get(int id);
		Color& operator[](int id);
		Color  operator[](int id) const;
		Layer& operator=(const Layer& source);

		Layer& fill(Color fColor, int from = 0, int to = -1);

		Layer& merge_overlay(const Layer &top, int offset = 0, bool wrap = false);
		Layer& merge_multiply(const Layer &top, int offset = 0, bool wrap = false);
		Layer& merge_multiply(const std::vector<uint8_t> &scalars, int offset=0, bool wrap = false);
		Layer& merge_add(const Layer &top, int offset = 0, bool wrap = false);

		Layer& alpha_set(const std::vector<uint8_t> &newAlphas);

//		Layer& calculate_overlay(const Layer &top, int offset = 0, bool wrap = false) const;
//		Layer& calculate_multiply(const Layer &top, int offset = 0, bool wrap = false) const;
//		Layer& calculate_multiply(const uint8_t *scalars) const;
//		Layer& calculate_add(const Layer &top, int offset = 0, bool wrap = false) const;
	};
}

#endif
