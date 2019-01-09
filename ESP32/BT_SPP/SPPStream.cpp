/*
 * SPPStream.cpp
 *
 *  Created on: 25 Nov 2018
 *      Author: xasin
 */

#include "SPPStream.h"
#include "SPPServer.h"

namespace Bluetooth {

SPP_Stream::SPP_Stream(SPP_Server &server, uint16_t id) :
	SPP_Value(server, id),
	streamBuffer(), write_callback(nullptr) {

	maxCapacity = 30;
}

void SPP_Stream::on_reconnect() {
	for(std::string& s : streamBuffer) {
		this->write(s.data(), s.length());
	}

	streamBuffer.clear();
}

void SPP_Stream::accept_data(const void *data, size_t length) {
	if(write_callback != nullptr) {
		std::string dataElement = std::string(reinterpret_cast<const char*>(data), length);
		write_callback(dataElement);
	}
}

void SPP_Stream::push(const std::string &data) {
	if(server.is_connected())
		this->write(data.data(), data.length());
	else {
		if(streamBuffer.size() >= maxCapacity)
			streamBuffer.erase(streamBuffer.begin());
		streamBuffer.push_back(data);
	}
}
void SPP_Stream::push(const void *data, size_t length) {
	std::string dataString = std::string(reinterpret_cast<const char *>(data), length);

	this->push(dataString);
}

} /* namespace Bluetooth */
