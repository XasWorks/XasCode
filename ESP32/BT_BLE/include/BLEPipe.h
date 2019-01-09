/*
 * BLEPipe.h
 *
 *  Created on: 9 Jan 2019
 *      Author: xasin
 */

#ifndef XASLIBS_BT_BLE_BLEPIPE_H_
#define XASLIBS_BT_BLE_BLEPIPE_H_

#include "BLEHandler.h"
#include "BasePipe.h"

namespace Xasin {
namespace Communication {

class BLE_Pipe: public BasePipe, private Peripheral::BLE_Handler {
public:
	BLE_Pipe(const char *name);
};

} /* namespace Communication */
} /* namespace Xasin */

#endif /* XASLIBS_BT_BLE_BLEPIPE_H_ */
