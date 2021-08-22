

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

const char * msgs[] = {
	"one",
	"two",
	"three"
};
int msg_point = 0;

static uint16_t gconn_handle = 0xFFFF;
static uint16_t gattr_handle = 0xFFFF;

// int gatt_access_fn(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg) {

// 	gconn_handle = conn_handle;
// 	gattr_handle = attr_handle;

// 	ESP_LOGI(tag, "Got a %s!", ctxt->op == BLE_GATT_ACCESS_OP_READ_CHR ? "read" : "write");

// 	if(ctxt->op == BLE_GATT_ACCESS_OP_READ_CHR) {
// 		auto m = msgs[msg_point++ % 3];
// 		os_mbuf_append(ctxt->om, m, strlen(m));
// 	}
// 	else if(ctxt->op == BLE_GATT_ACCESS_OP_WRITE_CHR) {
// 		auto len = OS_MBUF_PKTLEN(ctxt->om);

// 		uint8_t * print_buffer = new uint8_t[len];

// 		ble_hs_mbuf_to_flat(ctxt->om, print_buffer, len, nullptr);

// 		ESP_LOGI(tag, "Got message: %s", print_buffer);

// 		delete print_buffer;
// 	}

// 	return 0;
// }

Server::Server() : services() {

	// ble_uuid128_t * tUUID = new ble_uuid128_t({
	// 	BLE_UUID_TYPE_128,
	// 	{
	// 		0x53, 0xfe, 0xf2, 0x10, 
	// 		0x9e, 0xaa, 0x4d, 0x0b, 0xa3, 0x7f, 
	// 		0x49, 0x20, 0x77, 0x6b, 0xc1, 0xb6
	// 	}
	// });

	// ble_uuid128_t * tUUID2 = new ble_uuid128_t({
	// 	BLE_UUID_TYPE_128,
	// 	{
	// 		0x53, 0xfe, 0xf2, 0x11, 
	// 		0x9e, 0xaa, 0x4d, 0x0b, 0xa3, 0x7f, 
	// 		0x49, 0x20, 0x77, 0x6b, 0xc1, 0xb6
	// 	}
	// });

	// ble_gatt_chr_def * tCHR = new ble_gatt_chr_def[2] {
	// 	{
	// 		reinterpret_cast<ble_uuid_t*>(tUUID2),
	// 		gatt_access_fn,
	// 		nullptr,
	// 		nullptr,
	// 		BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE | BLE_GATT_CHR_F_NOTIFY,
	// 		0,
	// 		0,
	// 	},
	// 	{
	// 		0
	// 	}
	// };

	// services.push_back((ble_gatt_svc_def) {
	// 	BLE_GATT_SVC_TYPE_PRIMARY,
	// 	reinterpret_cast<ble_uuid_t*>(tUUID),
	// 	nullptr,
	// 	tCHR,
	// });

	// Append an empty element to the very back that serves as 
	// terminator for the nimBLE stack.
	services.push_back({});
}

void Server::append_service(Service & service) {
	service.local_service_id = services.size() - 1;
	services.push_back({});
}

void ble_host_task(void *args) {
	nimble_port_run();
	nimble_port_freertos_deinit();
}

void Server::init() {
	ESP_LOGI(tag, "Initializing NimBLE GATT Server!");
	
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
	ESP_LOGI(tag, "Starting advertisement");

	while(!ble_hs_synced())
		vTaskDelay(10);
	
	auto rc = ble_hs_util_ensure_addr(0);
	assert(rc == 0);

	uint8_t own_addr_type;
	/* Figure out address to use while advertising (no privacy for now) */
	rc = ble_hs_id_infer_auto(0, &own_addr_type);
	if (rc != 0) {
		ESP_LOGE(tag, "Error determining address type; rc=%d\n", rc);
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
		BLE_HS_FOREVER, &ad, nullptr, nullptr
		);
	
	if(rc != 0)
		ESP_LOGE(tag, "BLE GAP Start error: %d", rc);
}

void Server::DBG_send_str(const char * str) {
	//ble_gattc_notify_custom(gconn_handle, gattr_handle, ble_hs_mbuf_from_flat(str, strlen(str)));
}

}
}