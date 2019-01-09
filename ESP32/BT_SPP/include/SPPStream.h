/*
 * SPPStream.h
 *
 *  Created on: 25 Nov 2018
 *      Author: xasin
 */

#ifndef COMPONENTS_BT_SPP_SPPSTREAM_H_
#define COMPONENTS_BT_SPP_SPPSTREAM_H_

#include <string>
#include <vector>
#include <functional>

#include "SPPValue.h"

namespace Bluetooth {

class SPP_Stream : public SPP_Value {
private:
	std::vector<std::string> streamBuffer;

	void accept_data(const void *data, size_t length);
	void on_reconnect();

public:
	size_t maxCapacity;

	std::function<void (std::string &inElement)> write_callback;

	SPP_Stream(SPP_Server &server, uint16_t id);

	void push(const std::string &data);
	void push(const void *data, size_t length);
};

} /* namespace Bluetooth */

#endif /* COMPONENTS_BT_SPP_SPPSTREAM_H_ */
