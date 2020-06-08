
#include "NeoController/Layer.h"

namespace Xasin {

Layer::Layer(const int length) : colors(length) {
	alpha = 1;

	fill(0);
}
Layer::Layer(const Layer &source) : colors(source.colors) {
	alpha = source.alpha;
}
Layer::Layer(Color color) : Layer(1) {
	colors[0] = color;
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

	if(!wrap) {
		if(from < 0)
			from = 0;
		if(to > length())
			to = length();
	}

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
Layer& Layer::merge_transition(const Layer &top, int offset, bool wrap) {
	int from = offset;
	int to   = offset + top.length();

	if(!wrap) {
		if(from < 0)
			from = 0;
		if(to > length())
			to = length();
	}

	int top_count = (from - offset) % top.length();
	if(top_count < 0)
		top_count += top.length();

	const Color *from_ptr = &top.colors.front() + top_count;
	const Color *from_ptr_end = &top.colors.back() + 1;
	const Color *from_ptr_start = &top.colors.front();
	Color *to_ptr = &this->get(from);
	Color *to_ptr_end = &colors.back() + 1;
	Color *to_ptr_start = &colors.front();

	for(int i=from; i<to; i++) {
		to_ptr->merge_transition(*from_ptr, top.alpha);
		if(++to_ptr >= to_ptr_end)
			to_ptr = to_ptr_start;
		if(++from_ptr >= from_ptr_end)
			from_ptr = from_ptr_start;
	}

	return *this;
}

Layer& Layer::alpha_set(const std::vector<float> &newAlphas) {
	int to = newAlphas.size();
	if(to > this->length())
		to = this->length();

	for(int i=0; i<to; i++)
		colors[i].alpha = newAlphas[i];

	return *this;
}

}
