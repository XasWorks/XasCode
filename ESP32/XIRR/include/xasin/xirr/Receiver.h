/*
 * Receiver.h
 *
 *  Created on: 7 Jun 2019
 *      Author: xasin
 */

#ifndef ESP32_XIRR_RECEIVER_H_
#define ESP32_XIRR_RECEIVER_H_

#include "driver/rmt.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <functional>

#include <vector>

namespace Xasin {
namespace XIRR {

class Receiver {
private:
	const gpio_num_t rxPin;
	const rmt_channel_t rmtChannel;

	TaskHandle_t rxTaskHandle;


	rmt_item32_t *currentItem;
	uint32_t itemOffset;
	size_t numItems;
	bool get_next_bit();
	void parse_item(rmt_item32_t *item, size_t num);

public:
	Receiver(gpio_num_t pin, rmt_channel_t channel);
	~Receiver();

	void init();

	std::function<void(const uint8_t *data, uint8_t len, uint8_t channel)> on_rx;

	void _rx_task();
};

} /* namespace XIRR */
} /* namespace Xasin */

#endif /* ESP32_XIRR_RECEIVER_H_ */
