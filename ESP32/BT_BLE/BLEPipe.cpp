/*
 * BLEPipe.cpp
 *
 *  Created on: 9 Jan 2019
 *      Author: xasin
 */

#include "BLEPipe.h"

namespace Xasin {
namespace Communication {

BLE_Pipe::BLE_Pipe(const char *name) : BasePipe(),
		ble_connection(nullptr), ble_service(nullptr),
		ble_characteristic(nullptr) {
}

void BLE_Pipe::start() {
	ble_connection 	= new Peripheral::BLE_Handler("TestName");
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
