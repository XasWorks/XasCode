/*
 * Characteristic.h
 *
 *  Created on: 9 Oct 2018
 *      Author: xasin
 */

#ifndef COMPONENTS_BLUETOOTH_CHARACTERISTIC_H_
#define COMPONENTS_BLUETOOTH_CHARACTERISTIC_H_

#include <functional>

#include "esp_gatts_api.h"
#include "esp_bt.h"

namespace Xasin {
class BLE_Handler;

namespace Bluetooth {

class Service;

class Characteristic {
protected:
	friend BLE_Handler;

	uint16_t attr_handle;

	virtual void read_reply(esp_ble_gatts_cb_param_t::gatts_read_evt_param readEvent);
	virtual void handle_write(esp_ble_gatts_cb_param_t::gatts_write_evt_param *writeEvent);

public:
	struct write_dataset {
		uint16_t length;
		void *data;
	};
	typedef esp_ble_gatts_cb_param_t::gatts_read_evt_param read_dataset;

	Service * const service;

	esp_bt_uuid_t id;
	esp_gatt_perm_t 		perm;
	esp_gatt_char_prop_t 	prop;
	esp_attr_control_t 		autoResp;

	esp_attr_value_t 		value;

	bool is_descriptor;

	std::function<void (write_dataset)> write_cb;
	std::function<void (read_dataset)>  read_cb;

	Characteristic(Service * headService);

	void set_uuid16(uint16_t uuid);
	void set_uuid32(uint32_t uuid);

	void set_value(void *data, uint8_t len, uint8_t max_len);
	void set_value(void *data, uint8_t len);

	void can_read(bool val);
	void can_write(bool val);
	void can_notify(bool val);
	void can_indicate(bool val);

	void serve_read(read_dataset readEvent, void const * data, uint8_t dataLen);

	void send_notify(void *data, uint8_t data_len, bool need_confirm);

	void notify();
	void indicate();

	bool is_uuid(esp_bt_uuid_t &id);
};

} /* namespace Bluetooth */
} /* namespace Peripheral */

#endif /* COMPONENTS_BLUETOOTH_CHARACTERISTIC_H_ */
