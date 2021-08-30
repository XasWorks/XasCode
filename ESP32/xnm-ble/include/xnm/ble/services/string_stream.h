
#pragma once

#include "../../ble.h"

namespace XNM {
namespace BLE {

static ble_uuid128_t strstream_chr_uuid =
	XNM_BLE_UUID(0x00, 0x01, 0x00, 0x00);

static ble_uuid128_t logstream_srv_uuid = 
	XNM_BLE_UUID(0x0F, 0x00, 0x00, 0x01);

class StringStream : public Service {
private:
	ble_uuid128_t chr_uuid;

	std::vector<char> in_buffer;

	uint16_t attr_handle;
	ble_gatt_chr_def stream_chr[2];

	static int strstream_fn(uint16_t conn_handle, uint16_t attr_handle, ble_gatt_access_ctxt *ctxt, void *arg);

public : StringStream(Server &server, ble_uuid128_t srv_uuid = logstream_srv_uuid, ble_uuid128_t chr_uuid = logstream_srv_uuid);

	// Will send the given string over a BLE channel.
	// The string may be "chopped" into subsections across multiple
	// BLE packets to fit MTU restrictions, with the final 
	// packet being null-terminated.
	void send_str(const char *string);

	// Will be called once a full string packet has been received, i.e.
	// the received packet ends in a zero.
	// Zero-Termination will be included in the string!
	std::function<void (std::vector<char> &&)> on_receive;
};

}
}