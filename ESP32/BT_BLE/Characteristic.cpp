/*
 * Characteristic.cpp
 *
 *  Created on: 9 Oct 2018
 *      Author: xasin
 */

#include "Characteristic.h"
#include "Service.h"
#include "BLEHandler.h"

namespace Xasin 	{
namespace Bluetooth {

void Characteristic::read_reply(read_dataset readEvent) {
	if(read_cb != nullptr)
		read_cb(readEvent);
	else
		this->serve_read(readEvent, value.attr_value, value.attr_len);
}
void Characteristic::handle_write(esp_ble_gatts_cb_param_t::gatts_write_evt_param *writeEvent) {
	if(write_cb != nullptr) {
		write_dataset data = {};
		data.data 	= writeEvent->value;
		data.length = writeEvent->len;

		write_cb(data);
	}

	if(writeEvent->need_rsp)
		esp_ble_gatts_send_response(service->handler->GATT_if, writeEvent->conn_id, writeEvent->trans_id, ESP_GATT_OK, nullptr);
}

Characteristic::Characteristic(Service * service) :
		attr_handle(0),
		service(service),
		id(), perm(), prop(), autoResp(), value(),
		is_descriptor(false) {

	write_cb = nullptr;
	read_cb  = nullptr;

	can_read(true);

	autoResp.auto_rsp = 0;

	value.attr_len = 0;
	value.attr_max_len = 0;
	value.attr_value = nullptr;
}

void Characteristic::set_uuid16(uint16_t uuid) {
	id.len = ESP_UUID_LEN_16;
	id.uuid.uuid16 = uuid;
}
void Characteristic::set_uuid32(uint32_t uuid) {
	id.len = ESP_UUID_LEN_32;
	id.uuid.uuid32 = uuid;
}

void Characteristic::set_value(void *data, uint8_t len, uint8_t max_len) {
	value.attr_len = len;
	value.attr_max_len = max_len;
	value.attr_value = reinterpret_cast<uint8_t *>(data);
}
void Characteristic::set_value(void *data, uint8_t len) {
	value.attr_len = len;
	value.attr_value = reinterpret_cast<uint8_t *>(data);
}

void Characteristic::can_read(bool val) {
	if(val) {
		perm |= ESP_GATT_PERM_READ;
		prop |= ESP_GATT_CHAR_PROP_BIT_READ;
	}
	else {
		perm &= ~ESP_GATT_PERM_READ;
		prop &= ~ESP_GATT_CHAR_PROP_BIT_READ;
	}
}
void Characteristic::can_write(bool val) {
	if(val) {
		perm |= ESP_GATT_PERM_WRITE;
		prop |= ESP_GATT_CHAR_PROP_BIT_WRITE;
	}
	else {
		perm &= ~ESP_GATT_PERM_WRITE;
		prop &= ~ESP_GATT_CHAR_PROP_BIT_WRITE;
	}
}
void Characteristic::can_indicate(bool val) {
	auto b = ESP_GATT_CHAR_PROP_BIT_INDICATE;
	if(val)
		prop |= b;
	else
		prop &= ~b;
}
void Characteristic::can_notify(bool val) {
	auto b = ESP_GATT_CHAR_PROP_BIT_NOTIFY;
	if(val)
		prop |= b;
	else
		prop &= ~b;
}

void Characteristic::serve_read(read_dataset readEvent, void const * data, uint8_t dataLen) {
	esp_gatt_rsp_t response = {};
	response.handle = readEvent.handle;

	auto rspData = &response.attr_value;
	rspData->auth_req = 0;
	rspData->handle   = readEvent.handle;
	rspData->len      = dataLen;
	rspData->offset	 = 0;

	memcpy(rspData->value, data, dataLen);

	esp_ble_gatts_send_response(service->handler->GATT_if, readEvent.conn_id, readEvent.trans_id, ESP_GATT_OK, &response);
}

void Characteristic::send_notify(void *data, uint8_t data_len, bool need_confirm) {
	auto h = service->handler;
	esp_ble_gatts_send_indicate(h->GATT_if, h->connection_id, attr_handle, data_len, reinterpret_cast<uint8_t *>(data), need_confirm);
}

void Characteristic::notify() {
	send_notify(value.attr_value, value.attr_len, false);
}
void Characteristic::indicate() {
	send_notify(value.attr_value, value.attr_len, true);
}

bool Characteristic::is_uuid(esp_bt_uuid_t &tId) {
	if(tId.len != id.len)
		return false;

	return memcmp(&id.uuid, &tId.uuid, id.len) == 0;
}

} /* namespace Bluetooth */
} /* namespace Peripheral */
