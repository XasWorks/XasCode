/*
 * Transmitter.h
 *
 *  Created on: 3 Jun 2019
 *      Author: xasin
 */

#ifndef ESP32_XIRR_TRANSMITTER_H_
#define ESP32_XIRR_TRANSMITTER_H_

#include "xasin/BLDCHandler.h"

#include "driver/rmt.h"

#include "esp_pm.h"
#include "esp32/pm.h"

#include <string>
#include <array>
#include <vector>

namespace Xasin {
namespace Drone {

class DShot : public BLDCHandler {
private:
	gpio_num_t currentTXPin;
	const rmt_channel_t rmtChannel;

	esp_pm_lock_handle_t powerLock;

	std::array<rmt_item32_t, 20> rmt_buffer;

	std::vector<float> throttle_values;

	void raw_send(uint16_t throttle, bool telemetry = false, bool wait = false);
	void set_channel(uint8_t chNum);

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
		SPIN_3D		= 100, // FIXME
	};

	DShot(rmt_channel_t channel, uint8_t channel_count, uint8_t gpio_start);
	~DShot();

	void init();

	void	set_motor_power(uint8_t id, float throttle);
	float	get_motor_power(uint8_t id);

	void	send_cmd(uint8_t id, dshot_cmd_t cmd);
};

} /* namespace XIRR */
} /* namespace Xasin */

#endif /* ESP32_XIRR_TRANSMITTER_H_ */
