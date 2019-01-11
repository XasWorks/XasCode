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
bool BasePipe::send_packet(uint16_t id, Data_Packet data) {
	return false;
}

bool BasePipe::send_packet(uint16_t id, size_t length, const void *data) {
	return send_packet(id, {length, data});
}
bool BasePipe::send_packet(uint16_t id, const std::string &data) {
	return send_packet(id, {data.length(), data.data()});
}

} /* namespace Communication */
} /* namespace Xasin */
