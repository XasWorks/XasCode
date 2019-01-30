/*
 * BLEPipe.h
 *
 *  Created on: 9 Jan 2019
 *      Author: xasin
 */

#ifndef XASLIBS_BT_BLE_BLEPIPE_H_
#define XASLIBS_BT_BLE_BLEPIPE_H_

#include "../../ID_Coms/include/RegisterBlock.h"
#include "BLEHandler.h"
#include "Characteristic.h"
#include "Service.h"


namespace Xasin {
namespace Communication {

class BLE_SlaveChannel: public SlaveChannel {
private:
	BLE_Handler ble_connection;
	Bluetooth::Service		  ble_service;
	Bluetooth::Characteristic ble_characteristic;
	Bluetooth::Characteristic ble_char_descriptor;
	uint16_t ble_char_desc_data;

	std::array<uint8_t, 20> data_buffer;

public:
	BLE_SlaveChannel(const char *name, RegisterBlock &registerBlock);

	void start();

	void send_update(uint16_t ID, Data_Packet data);

	bool is_connected();
};

} /* namespace Communication */
} /* namespace Xasin */

#endif /* XASLIBS_BT_BLE_BLEPIPE_H_ */
