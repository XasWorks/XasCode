
#pragma once

#include <vector>
#include <functional>
#include <string>

#define XNM_BLE_UUID(a, b, c, d) (ble_uuid128_t({ \
	BLE_UUID_TYPE_128,						  \
	{										  \
		'm', 'n', 'x', 0x10, 0x9e, 0xaa, 	  \
		0x4d, 0x0b, 0xa3, 0x7f,               \
		0x49, 0x20,                           \
		d, c, a, b                            \
	}                                         \
}))

namespace XNM {
namespace BLE {
	static const char *TAG = "xnm-ble";
}
}

#include "ble/host.h"
#include "ble/service.h"

#include "ble/services/battery_svc.h"
#include "ble/services/string_stream.h"