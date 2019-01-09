/*
 * BLEPipe.cpp
 *
 *  Created on: 9 Jan 2019
 *      Author: xasin
 */

#include "BLEPipe.h"

namespace Xasin {
namespace Communication {

BLE_Pipe::BLE_Pipe(const char *name) : BasePipe(), BLE_Handler(name) {
}

} /* namespace Communication */
} /* namespace Xasin */
