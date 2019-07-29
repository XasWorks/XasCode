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
#include <array>

namespace Xasin {
namespace OneShot125 {

class DShot {
private:
	gpio_num_t currentTXPin;
	const rmt_channel_t rmtChannel;

	esp_pm_lock_handle_t powerLock;

	std::array<rmt_item32_t, 20> rmt_buffer;

	uint16_t current_throttle_value;

	void raw_send(uint16_t throttle, bool telemetry = false, bool wait = false);

public:
	enum dshot_cmd_t {
		STOP = 0,
		BEACON0,
		BEACON1,
		BEACON2,
		BEACON3,
		BEACON4,
		SPIN_NORMAL = 20,
		SPIN_REVERSE = 21,
	};

	DShot(gpio_num_t pin, rmt_channel_t channel);
	~DShot();

	void init();

	void send(uint16_t throttle);
	void send_cmd(dshot_cmd_t cmd);
};

} /* namespace XIRR */
} /* namespace Xasin */

#endif /* ESP32_XIRR_TRANSMITTER_H_ */
