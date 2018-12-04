
#include "Layer.h"

namespace Peripheral {

Layer::Layer(const int length) : colors(length) {
	alpha = 255;

	fill(0);
}
Layer::Layer(const Layer &source) : colors(source.colors) {
	alpha = source.alpha;
}

int Layer::length() const {
	return colors.size();
}

Color& Layer::get(int id) {
	id %= length();

	if(id < 0)
		id += length();

	return colors[id];
}

Color& Layer::operator[](int id) {
	return get(id);
}
Color Layer::operator[](int id) const {
	id %= length();
	if(id < 0)
		id += length();

	return colors[id];
}
Layer& Layer::operator=(const Layer& source) {
	this->colors = source.colors;
	this->alpha = source.alpha;

	return *this;
}

Layer& Layer::fill(Color fColor, int from, int to) {
	if(to == -1)
		to = length();

	if(from < 0)
		from = 0;
	if(to > length())
		to = length();

	if(from > to) {
		int temp = to;
		to = from;
		from = temp;
	}

	for(int i=from; i<to; i++)
		this->get(i) = fColor;

	return *this;
}

Layer& Layer::merge_overlay(const Layer &top, int offset, bool wrap) {
	int from = offset;
	int to   = offset + top.length();

//	if(!wrap) {
//		if(from < 0)
//			from = 0;
//		if(to > length())
//			to = length();
//	}

	for(int i=from; i<to; i++) {
		this->get(i).merge_overlay(top[i-offset], top.alpha);
	}

	return *this;
}
Layer& Layer::merge_multiply(const Layer &top, int offset, bool wrap) {
	int from = offset;
	int to   = offset + top.length();

	if(!wrap) {
		if(from < 0)
			from = 0;
		if(to > length())
			to = length();
	}

	for(int i=from; i<to; i++) {
		this->get(i).merge_multiply(top[i-offset], top.alpha);
	}

	return *this;
}
Layer& Layer::merge_multiply(const std::vector<uint8_t> &scalars, int offset, bool wrap) {
	int from = offset;
	int to   = offset + scalars.size();

	if(!wrap) {
		if(from < 0)
			from = 0;
		if(to > length())
			to = length();
	}


	for(int i=from; i<to; i++) {
		this->get(i).merge_multiply(scalars[i-offset]);
	}

	return *this;
}
Layer& Layer::merge_add(const Layer &top, int offset, bool wrap) {
	int from = offset;
	int to   = offset + top.length();

	if(!wrap) {
		if(from < 0)
			from = 0;
		if(to > length())
			to = length();
	}

	for(int i=from; i<to; i++) {
		this->get(i).merge_add(top[i-offset], top.alpha);
	}

	return *this;
}

Layer& Layer::alpha_set(const std::vector<uint8_t> &newAlphas) {
	int to = newAlphas.size();
	if(to > this->length())
		to = this->length();

	for(int i=0; i<to; i++)
		colors[i].alpha = newAlphas[i];

	return *this;
}

}
