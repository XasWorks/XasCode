#include "include/xnm/ble/services/battery_svc.h"

namespace XNM {
namespace BLE {

int bat_read_fn(uint16_t conn_handle, uint16_t attr_handle, ble_gatt_access_ctxt * ctxt, void *arg) {
	if(ctxt->op == BLE_GATT_ACCESS_OP_READ_CHR) {
		os_mbuf_append(ctxt->om, arg, 1);
	}
	
	return 0;
}

BatService::BatService(Server &server) : Service(server),
	bat_id({BLE_UUID_TYPE_16, 0x2A19}),
	bat_chr({
		{
			reinterpret_cast<ble_uuid_t*>(&bat_id),
			bat_read_fn, &bat_level,
			nullptr,
			BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_NOTIFY,
			0,

			&attr_handle
		},
		{ nullptr }
	}) {

	uuid.u16 = {
		BLE_UUID_TYPE_16, 
		0x180F
	};
	chr_pointer = bat_chr;
}

void BatService::update_bat(uint8_t level) {
	if(bat_level == level)
		return;

	bat_level = level;

	ble_gattc_notify(0, attr_handle);
}

}
}