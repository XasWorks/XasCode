/*
 * NeoController.h
 *
 *  Created on: 22 Feb 2020
 *      Author: xasin
 */

#ifndef STM32F4_NEOCONTROLLER_NEOCONTROLLER_H_
#define STM32F4_NEOCONTROLLER_NEOCONTROLLER_H_

#include "NeoController/Color.h"
#include "NeoController/Layer.h"

#include "stm32f3xx_hal.h"

#include <vector>

namespace Xasin {

class NeoController {
	SPI_HandleTypeDef * const spi;

	std::vector<uint8_t> write_buffer;
	uint8_t *current_data;

	void write_u24(uint32_t b);
	void write_u24(const Color &c);

public:
	const int length;
	Layer colors;

	NeoController(SPI_HandleTypeDef &spi, int num_leds);

	void push();
};

} /* namespace Xasin */

#endif /* STM32F4_NEOCONTROLLER_NEOCONTROLLER_H_ */
