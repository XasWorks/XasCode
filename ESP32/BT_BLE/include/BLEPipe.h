/*
 * BLEPipe.h
 *
 *  Created on: 9 Jan 2019
 *      Author: xasin
 */

#ifndef XASLIBS_BT_BLE_BLEPIPE_H_
#define XASLIBS_BT_BLE_BLEPIPE_H_

#include "BLEHandler.h"
#include "Characteristic.h"
#include "Service.h"

#include "BasePipe.h"

namespace Xasin {
namespace Communication {

class BLE_Pipe: public BasePipe {
private:
	Peripheral::BLE_Handler *ble_connection;
	Peripheral::Service		*ble_service;
	Peripheral::Characteristic *ble_characteristic;

public:
	BLE_Pipe(const char *name);

	void start();

//	bool is_connected();
//	bool send_packet(uint16_t id, Data_Packet data);
};

} /* namespace Communication */
} /* namespace Xasin */

#endif /* XASLIBS_BT_BLE_BLEPIPE_H_ */
