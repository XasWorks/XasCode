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
	Peripheral::BLE_Handler *ble_connection;
	Peripheral::Service		*ble_service;
	Peripheral::Characteristic *ble_characteristic;

public:
	BLE_SlaveChannel(const char *name, RegisterBlock &registerBlock);

	void start();

//	bool is_connected();
//	bool send_packet(uint16_t id, Data_Packet data);
};

} /* namespace Communication */
} /* namespace Xasin */

#endif /* XASLIBS_BT_BLE_BLEPIPE_H_ */
