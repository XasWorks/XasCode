
#pragma once

#include "BaseHandler.h"
#include "BaseOutput.h"
#include "BaseProperty.h"

#include <xnm/ble.h>

namespace XNM {
namespace PropertyPoint {

static const ble_uuid128_t ble_srv_uuid = 
	XNM_BLE_UUID(0x50, 0x50, 0, 0);
static const ble_uuid128_t ble_chr_uuid =
	XNM_BLE_UUID(0x50, 0x50, 0, 1);

class BLEOutput : public BaseOutput {
private:
	BLE::StringStream ble;

protected:
	void send_upd_json(const cJSON * item, BaseProperty &prop);

public:
	BLEOutput(Handler & handler, BLE::Server & ble);

	void init();
};

}
}