/*
 * BLEPipe.cpp
 *
 *  Created on: 9 Jan 2019
 *      Author: xasin
 */

#include "BLESlaveChannel.h"

namespace Xasin {
namespace Communication {

BLE_SlaveChannel::BLE_SlaveChannel(const char *name, RegisterBlock &registerBlock) : SlaveChannel(registerBlock),
		ble_connection(name), ble_service(&ble_connection),
		ble_characteristic(&ble_service),
		ble_char_descriptor(&ble_service), ble_char_desc_data(0),
		data_buffer() {
}

void BLE_SlaveChannel::start() {
	ble_connection.set_GAP_param(ble_connection.get_GAP_defaults());

	ble_characteristic.set_uuid32('XSPP');
	ble_char_descriptor.set_uuid16(0x2902);

	ble_characteristic.set_value(&ble_char_desc_data, 1);
	ble_char_descriptor.set_value(&ble_char_desc_data, 2, 2);

	ble_characteristic.can_write(true);
	ble_characteristic.can_read(true);
	ble_characteristic.can_indicate(true);

	ble_characteristic.write_cb = [this](Xasin::Bluetooth::Characteristic::write_dataset wD) {
		if(wD.length < 2)
			return;

		uint16_t ID = *reinterpret_cast<uint16_t*>(wD.data);

		printf("Got data for ID 0x%4X, length: %d\n", ID, wD.length -2);

		mainRegister.write_register(ID, {size_t(wD.length -2), wD.data + 2});
	};

	ble_char_descriptor.is_descriptor = true;
	ble_char_descriptor.can_write(true);

	ble_service.add_char(&ble_characteristic);
	ble_service.add_char(&ble_char_descriptor);

	ble_service.set_uuid32('XSPP');

	ble_connection.add_service(&ble_service);

	ble_connection.setup_GATTS();

	vTaskDelay(1000);
	ble_connection.start_advertising();
}

void BLE_SlaveChannel::send_update(uint16_t ID, Data_Packet data) {
	if(data.length > 18)
		return;

	*reinterpret_cast<uint16_t*>(data_buffer.data()) = ID;
	memcpy(data_buffer.data() +2, data.data, data.length);

	ble_characteristic.send_notify(data_buffer.data(), data.length +2, false);
}

bool BLE_SlaveChannel::is_connected() {
	return ble_connection.is_connected();
}
} /* namespace Communication */
} /* namespace Xasin */
