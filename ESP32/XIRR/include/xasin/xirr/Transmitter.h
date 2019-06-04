/*
 * Transmitter.h
 *
 *  Created on: 3 Jun 2019
 *      Author: xasin
 */

#ifndef ESP32_XIRR_TRANSMITTER_H_
#define ESP32_XIRR_TRANSMITTER_H_

#include "driver/rmt.h"

#include "esp_pm.h"
#include "esp32/pm.h"

#include <string>
#include <vector>

namespace Xasin {
namespace XIRR {

class Transmitter {
private:
	const gpio_num_t txPin;
	const rmt_channel_t rmtChannel;

	esp_pm_lock_handle_t powerLock;

	std::vector<rmt_item32_t> rmt_buffer;

	void add_byte(uint8_t data, uint8_t cont);

public:
	Transmitter(gpio_num_t pin, rmt_channel_t channel);
	~Transmitter();

	void init();

	void send(const void *data, size_t length, uint8_t channel);

	template <typename T>
	void send(const T &data, uint8_t channel) {
		send(&data, sizeof(T), channel);
	};
};

} /* namespace XIRR */
} /* namespace Xasin */

#endif /* ESP32_XIRR_TRANSMITTER_H_ */
