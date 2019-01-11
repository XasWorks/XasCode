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
		ble_connection(nullptr), ble_service(nullptr),
		ble_characteristic(nullptr) {
}

void BLE_SlaveChannel::start() {
	ble_connection 	= new Peripheral::BLE_Handler("TestName");

	ble_connection->set_GAP_param(ble_connection->get_GAP_defaults());

	ble_service		= new Peripheral::Service(ble_connection);
	ble_characteristic = new Peripheral::Characteristic(ble_service);

	ble_characteristic->set_uuid32('XSPP');
	ble_service->set_uuid32('XSPP');

	ble_characteristic->can_write(true);
	ble_characteristic->can_read(true);
	ble_characteristic->can_indicate(true);

	ble_service->add_char(ble_characteristic);
	ble_connection->add_service(ble_service);

	ble_connection->setup_GATTS();
	ble_connection->start_advertising();
}

} /* namespace Communication */
} /* namespace Xasin */
