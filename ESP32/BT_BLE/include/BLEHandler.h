/*
 * BLEHandler.h
 *
 *  Created on: 8 Oct 2018
 *      Author: xasin
 */

#ifndef COMPONENTS_BLUETOOTH_BLEHANDLER_H_
#define COMPONENTS_BLUETOOTH_BLEHANDLER_H_

#include <cstring>

#include "assert.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_system.h"

#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_bt_defs.h"
#include "esp_bt_device.h"

#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"

#include "Service.h"

namespace Xasin {
using namespace Bluetooth;

class BLE_Handler {
protected:
	friend Characteristic;
	friend Service;

	static BLE_Handler *masterHandler;

	static void GAP_Callback(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param);
	static void GATTs_Callback(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);

	enum BT_STATUS {
		UNINITIALIZED,
		DISABLED,
		STARTING,
		IDLE,
		ADVERTISING,
		DISCONNECTED,
		CONNECTED,
	};
	volatile BT_STATUS BT_status;
	volatile BT_STATUS BT_status_target;

	uint64_t adv_stop_time;

	TaskHandle_t powerTask_handle;

	esp_bd_addr_t connected_device;
	uint16_t	  connection_id;

	esp_ble_adv_data_t GAP_param;
	esp_ble_adv_data_t GAP_param_rsp;

	esp_gatt_if_t GATT_if;

	std::vector<Service *> services;

	const char *name;

	void set_BT_status(BT_STATUS newStat);

	void process_GAP(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param);
	void process_GATTs(esp_gatts_cb_event_t event, esp_gatt_if_t iface, esp_ble_gatts_cb_param_t *param);

	void register_service(Service *service);

public:
	uint64_t client_connection_time;

	void power_task();

	BLE_Handler(const char *name);

	bool is_connected();

	esp_ble_adv_data_t get_GAP_defaults();
	void set_GAP_param(esp_ble_adv_data_t params);

	void add_service(Service * newService);

	void setup_GATTS();

	void enable();
	void disable();

	void disconnect();

	void start_advertising(uint64_t disableAfter = 0);
	void stop_advertising();
};

} /* namespace Peripheral */

#endif /* COMPONENTS_BLUETOOTH_BLEHANDLER_H_ */
