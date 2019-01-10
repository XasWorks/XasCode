/*
 * BasePipe.cpp
 *
 *  Created on: 9 Jan 2019
 *      Author: xasin
 */

#include "BasePipe.h"

namespace Xasin {
namespace Communication {

BasePipe::BasePipe()
	: on_connection_change(), on_received() {
}

bool BasePipe::is_connected() {
	return false;
}
bool BasePipe::send_packet(int id, const void *data, size_t length) {
	return false;
}

} /* namespace Communication */
} /* namespace Xasin */
