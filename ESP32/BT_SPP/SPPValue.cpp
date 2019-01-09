/*
 * SPPValue.cpp
 *
 *  Created on: 17 Nov 2018
 *      Author: xasin
 */

#include "SPPValue.h"
#include "SPPServer.h"

#include <cstring>

namespace Bluetooth {

SPP_Value::SPP_Value(SPP_Server& server, uint16_t id) :
		server(server),
		id(id) {

	server.values[id] = this;
}

void SPP_Value::on_reconnect() {
}
void SPP_Value::accept_data(const void *data, size_t length) {
}


bool SPP_Value::write(const void *data, size_t length) {
	if(!server.is_connected())
		return false;

	if(data != nullptr && length != 0)
		server.write_packet(id, data, length);
	else
		server.write_packet(id, 0, 0);

	return true;
}

} /* namespace Bluetooth */
