/*
 * NeoController.cpp
 *
 *  Created on: 22 Feb 2020
 *      Author: xasin
 */

#include <NeoController/NeoController.h>

#include <array>

namespace Xasin {

NeoController::NeoController(SPI_HandleTypeDef &spi, int num_leds, bool invert) :
	inv_output(invert),
	spi(&spi), write_buffer(12*num_leds), length(num_leds), colors(num_leds) {

	rg_swap_map = nullptr;
}

void NeoController::write_u24(uint32_t b) {
	for(int i=0; i<12; i++) {
		uint8_t out_b = 0b00010010;
		uint8_t mark = 0b100;

		while(mark) {
			if(b & (1<<23))
				out_b |= mark;
			b <<= 1;
			mark <<= 3;
		}

		if(inv_output)
			*current_data = !out_b;
		else
			*current_data = out_b;

		current_data++;
	}
}

void NeoController::write_u24(const Color &c, bool swap) {
	if(current_data == nullptr)
		return;

	auto c_data = c.getLEDValue();
	if(swap) {
		auto tmp = c_data.r;
		c_data.r = c_data.g;
		c_data.g = tmp;
	}

	write_u24(c_data.data);
}

void NeoController::push() {
	current_data = reinterpret_cast<uint8_t* >(write_buffer.data());

	if(rg_swap_map == nullptr) {
		for(int i=0; i<length; i++) {
			write_u24(colors[i]);
		}
	}
	else {
		for(int i=0; i<length; i++) {
			write_u24(colors[i], (rg_swap_map[i/32] >> (i&31)) & 1);
		}
	}

	HAL_SPI_Transmit_IT(spi, write_buffer.data(), length*12);
}

} /* namespace Xasin */
