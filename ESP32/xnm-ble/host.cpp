
#define __XNM_BLE_EXCLUDE_DIY_TYPES

#include "include/xnm/ble.h"

#include "esp_nimble_hci.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"

#include "host/ble_hs.h"
#include "host/util/util.h"

#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"

#include <esp_log.h>

namespace XNM {
namespace BLE {

int Server::static_gap_cb(ble_gap_event *event, void *arg) {
	assert(arg);

	return reinterpret_cast<Server*>(arg)->gap_cb(event);
}
void ble_host_task(void *args) {
	nimble_port_run();
	nimble_port_freertos_deinit();
}

Server::Server() : services() {
	// Append an empty element to the very back that serves as 
	// terminator for the nimBLE stack.
	services.push_back({});
}

int Server::gap_cb(ble_gap_event * event) {
	switch(event->type) {
		default: 
			ESP_LOGD(TAG, "Unused GAP event %d", event->type);
		break;

		case BLE_GAP_EVENT_CONNECT:
			if(event->connect.status == 0) {
				ESP_LOGI(TAG, "Connected!");
				is_connected = true;
			}
			else {
				is_connected = false;

				if(should_advertise)
					start_advertising();
			}
		break;

		case BLE_GAP_EVENT_MTU:
			mtu = event->mtu.value;
			ESP_LOGD(TAG, "New MTU is: %d", mtu);
		break;

		case BLE_GAP_EVENT_DISCONNECT:
			ESP_LOGI(TAG, "Disconnected!");

			if(should_advertise)
				start_advertising();
		break;
	}

	return 0;
}

void Server::append_service(Service & service) {
	service.local_service_id = services.size() - 1;
	services.push_back({});
}

void Server::init() {
	ESP_LOGI(TAG, "Initializing NimBLE GATT Server!");
	
	int ret = esp_nimble_hci_and_controller_init();

	if (ret != ESP_OK) {
		ESP_LOGE(TAG, "esp_nimble_hci_and_controller_init() failed with error: %d", ret);
		return;
	}

	nimble_port_init();

	memset(&ble_hs_cfg, 0, sizeof(ble_hs_cfg));

	ble_hs_cfg.sm_io_cap = BLE_SM_IO_CAP_NO_IO;

	//Initialize the NimBLE Host configuration

   ble_svc_gap_init();
	ble_svc_gatt_init();

	ble_gatts_count_cfg(services.data());
	ble_gatts_add_svcs(services.data());

	nimble_port_freertos_init(ble_host_task);
}

void Server::start_advertising(const char * name) {
	ESP_LOGI(TAG, "Starting advertisement");

	should_advertise = true;

	while(!ble_hs_synced())
		vTaskDelay(10);
	
	auto rc = ble_hs_util_ensure_addr(0);
	assert(rc == 0);

	uint8_t own_addr_type;
	/* Figure out address to use while advertising (no privacy for now) */
	rc = ble_hs_id_infer_auto(0, &own_addr_type);
	if (rc != 0) {
		ESP_LOGE(TAG, "Error determining address type; rc=%d\n", rc);
		return;
	}


	ble_gap_adv_params ad = {};
	ble_hs_adv_fields fields = {};

	ad.conn_mode = BLE_GAP_CONN_MODE_UND;
	ad.disc_mode = BLE_GAP_DISC_MODE_GEN;

	fields.flags = BLE_HS_ADV_F_DISC_GEN | BLE_HS_ADV_F_BREDR_UNSUP;

	/* Indicate that the TX power level field should be included; have the
	* stack fill this value automatically.  This is done by assigning the
	* special value BLE_HS_ADV_TX_PWR_LVL_AUTO.
	*/
	fields.tx_pwr_lvl_is_present = 1;
	fields.tx_pwr_lvl = BLE_HS_ADV_TX_PWR_LVL_AUTO;

	fields.name = (uint8_t *)name;
   fields.name_len = strlen(name);
   fields.name_is_complete = 1;

	fields.appearance = 0x0703;
	fields.appearance_is_present = 1;

	fields.num_uuids16 = 1;
	fields.uuids16_is_complete = 1;

   ble_gap_adv_set_fields(&fields);

	rc = ble_gap_adv_start(own_addr_type, nullptr,
		BLE_HS_FOREVER, &ad, static_gap_cb, this);
	
	if(rc != 0)
		ESP_LOGE(TAG, "BLE GAP Start error: %d", rc);
}

}
}