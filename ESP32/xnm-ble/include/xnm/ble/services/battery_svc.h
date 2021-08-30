
#pragma once

#include "../service.h"

namespace XNM {
namespace BLE {

class BatService : public Service {
private:
	uint8_t bat_level;

	uint16_t attr_handle;

	ble_uuid16_t bat_id;
	ble_gatt_chr_def bat_chr[2];

public:
	BatService(Server & server);

	void update_bat(uint8_t level);
};

}
}