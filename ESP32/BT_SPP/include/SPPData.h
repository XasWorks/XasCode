/*
 * SPPData.h
 *
 *  Created on: 23 Nov 2018
 *      Author: xasin
 */

#ifndef COMPONENTS_BT_SPP_SPPDATA_H_
#define COMPONENTS_BT_SPP_SPPDATA_H_

#include "SPPValue.h"

#include <functional>
#include <cstring>

namespace Bluetooth {

class SPP_Server;

class SPP_Data: public SPP_Value {
private:
	bool data_changed;

	void accept_data(const void *data, size_t length);
	void on_reconnect();

public:
	void	*data;
	size_t 	data_length;

	bool allow_write;
	bool allow_partial_write;

	std::function<void (const void* data, size_t length)> write_callback;

	SPP_Data(SPP_Server &server, uint16_t id, void *data, size_t data_length);
	template<typename T>
	SPP_Data(SPP_Server &server, uint16_t id, T& data) : SPP_Data(server, id, &data, sizeof(T)){
	}

	SPP_Data(SPP_Server &server, uint16_t id);

	template<typename T>
	void set(T& data, bool allow_write = false, bool allow_partial = false) {
		this->data = &data;
		this->data_length = sizeof(data);

		this->allow_write = allow_write;
		this->allow_partial_write = allow_partial;
	}

	bool update();
	void update_r();

	template<typename T>
	bool update(const T& data) {
		if(sizeof(T) == data_length)
			memcpy(this->data, &data, sizeof(T));

		return update();
	}
	template<typename T>
	void update_r(const T& data) {
		if(sizeof(T) == data_length)
			memcpy(this->data, &data, sizeof(T));

		update_r();
	}
};

} /* namespace Peripheral */

#endif /* COMPONENTS_BT_SPP_SPPDATA_H_ */
