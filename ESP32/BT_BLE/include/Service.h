/*
 * Service.h
 *
 *  Created on: 9 Oct 2018
 *      Author: xasin
 */

#ifndef COMPONENTS_BLUETOOTH_SERVICE_H_
#define COMPONENTS_BLUETOOTH_SERVICE_H_

#include <vector>

#include "assert.h"
#include "esp_gatts_api.h"

#include "Characteristic.h"

namespace Xasin {
class BLE_Handler;

namespace Bluetooth {


class Service {
private:
	void register_characteristic(Characteristic *newCharacteristic);
protected:
	friend BLE_Handler;
	friend Characteristic;

	std::vector<Characteristic *> characteristics;
	BLE_Handler * const handler;

	uint16_t handle;

	void set_handle(uint16_t handle);

public:
	esp_gatt_srvc_id_t id;

	Service(BLE_Handler *handler);

	uint8_t get_no_handles();

	void set_primary(bool primary);
	void set_uuid16(uint16_t uuid);
	void set_uuid32(uint32_t uuid);

	void add_char(Characteristic * newChar);
};

} /* namespace Bluetooth */
} /* namespace Peripheral */

#endif /* COMPONENTS_BLUETOOTH_SERVICE_H_ */
