
#define __XNM_BLE_EXCLUDE_DIY_TYPES

#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG
#include <esp_log.h>

#include "include/xnm/ble/services/string_stream.h"

#include "host/ble_hs.h"

namespace XNM {
namespace BLE {

int StringStream::strstream_fn(uint16_t conn_handle, uint16_t attr_handle, ble_gatt_access_ctxt * ctxt, void *arg) {
	assert(arg != nullptr);
	
	auto obj = reinterpret_cast<StringStream*>(arg);

	if(ctxt->op == BLE_GATT_ACCESS_OP_WRITE_CHR) {
		auto len = OS_MBUF_PKTLEN(ctxt->om);
		
		
		auto prev_size = obj->in_buffer.size();
		obj->in_buffer.resize(obj->in_buffer.size() + len);

		ble_hs_mbuf_to_flat(ctxt->om, obj->in_buffer.data() + prev_size, len, nullptr);

		if(*(obj->in_buffer.rbegin()) == 0) {
			ESP_LOGD(TAG, "String got string data %s", obj->in_buffer.data());
			
			if(obj->on_receive)
				obj->on_receive(std::forward<std::vector<char>>(obj->in_buffer));

			obj->in_buffer.resize(0);
			obj->in_buffer.reserve(255);
		}
	}
	
	return 0;
}

StringStream::StringStream(Server &server, ble_uuid128_t srv_uuid, ble_uuid128_t chr_uuid) : Service(server),
	chr_uuid(chr_uuid),
	in_buffer(),
	attr_handle(0),
	stream_chr({
		{
			reinterpret_cast<ble_uuid_t*>(&this->chr_uuid),
			strstream_fn, this,
			nullptr,
			BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE | BLE_GATT_CHR_F_NOTIFY,
			0,

			&attr_handle
		},
		{ nullptr }
	}) {

	this->uuid.u128 = srv_uuid;
	chr_pointer = stream_chr;
}

void StringStream::send_str(const char *str) {
	if(attr_handle == 0)
		return;

	ble_gattc_notify_custom(0, attr_handle, ble_hs_mbuf_from_flat(str, strlen(str) + 1));
}

}
}