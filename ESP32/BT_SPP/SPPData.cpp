/*
 * SPPData.cpp
 *
 *  Created on: 23 Nov 2018
 *      Author: xasin
 */

#include "SPPData.h"
#include "SPPServer.h"

namespace Bluetooth {

SPP_Data::SPP_Data(SPP_Server &server, uint16_t id, void *data, size_t data_length) :
	SPP_Value(server, id),
	data_changed(false),
	data(data), data_length(data_length),
	allow_write(false), allow_partial_write(false),
	write_callback(nullptr) {
}
SPP_Data::SPP_Data(SPP_Server &server, uint16_t id) :
	SPP_Data(server, id, nullptr, 0){
}

void SPP_Data::accept_data(const void *data, size_t length) {
	if(allow_write) {
		if(length > data_length)
			return;

		if((!allow_partial_write) && (length < data_length))
			return;

		memcpy(this->data, data, length);
	}

	if(write_callback != nullptr)
		write_callback(data, length);
}

void SPP_Data::on_reconnect() {
	if(data_changed) {
		data_changed = false;
		write(data, data_length);
	}
}

bool SPP_Data::update() {
	return this->write(data, data_length);
}
void SPP_Data::update_r() {
	if(!update())
		data_changed = true;
}


} /* namespace Peripheral */
