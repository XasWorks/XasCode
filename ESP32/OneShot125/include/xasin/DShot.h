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

#define DSHOT_MIN_THROTTLE 0.023

namespace Xasin {
namespace Drone {

class DShot : public BLDCHandler {
private:
	const rmt_channel_t rmtChannel_start;

	esp_pm_lock_handle_t powerLock;

	std::array<rmt_item32_t, 20> rmt_buffer;

	std::vector<float> throttle_values;
	std::vector<int>   direction_switch_counter;

	void raw_send(int id, uint16_t throttle, bool telemetry = false, bool wait = false);

	void init_channel(int id);

public:
	enum dshot_cmd_t {
		STOP = 0,
		BEACON0,
		BEACON1,
		BEACON2,
		BEACON3,
		BEACON4,
		SPIN_3D		= 10,
		SAVE_SETTING = 12,
		SPIN_NORMAL = 20,
		SPIN_REVERSE = 21,
	};

	DShot(rmt_channel_t channel_start, uint8_t channel_count, uint8_t gpio_start);
	~DShot();

	void init();

	void	set_motor_power(uint8_t id, float throttle);
	float	get_motor_power(uint8_t id);

	void	send_cmd(uint8_t id, dshot_cmd_t cmd);
};

} /* namespace XIRR */
} /* namespace Xasin */

#endif /* ESP32_XIRR_TRANSMITTER_H_ */
