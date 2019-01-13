/*
 * Service.cpp
 *
 *  Created on: 9 Oct 2018
 *      Author: xasin
 */

#include "Service.h"
#include "BLEHandler.h"

namespace Xasin		{
namespace Bluetooth {

void Service::register_characteristic(Characteristic *nC) {
	assert(handle != 0);
	if(nC->is_descriptor) {
		auto ret = esp_ble_gatts_add_char_descr(handle, &(nC->id), nC->perm, &(nC->value), &(nC->autoResp));
		ESP_ERROR_CHECK(ret);
	}
	else {
		auto ret = esp_ble_gatts_add_char(handle, &(nC->id), nC->perm, nC->prop, &(nC->value), &(nC->autoResp));
		ESP_ERROR_CHECK(ret);
	}
}

uint8_t Service::get_no_handles() {
	return characteristics.size()*2 + 2;
}

void Service::set_handle(uint16_t newHandle) {
	assert(this->handle == 0);
	this->handle = newHandle;

	for(auto c: characteristics)
		register_characteristic(c);

	esp_ble_gatts_start_service(handle);
}

Service::Service(BLE_Handler * handler) :
		characteristics(0), handler(handler),
		handle(0) {

	set_primary(true);
}

void Service::set_primary(bool primary) {
	id.is_primary = primary;
}
void Service::set_uuid16(uint16_t uuid) {
	id.id.uuid.len = ESP_UUID_LEN_16;
	id.id.uuid.uuid.uuid16 = uuid;
}
void Service::set_uuid32(uint32_t uuid) {
	id.id.uuid.len = ESP_UUID_LEN_32;
	id.id.uuid.uuid.uuid32 = uuid;
}

void Service::add_char(Characteristic *newChar) {
	assert(handle == 0);

	characteristics.push_back(newChar);
}

} /* namespace Bluetooth */
} /* namespace Peripheral */
