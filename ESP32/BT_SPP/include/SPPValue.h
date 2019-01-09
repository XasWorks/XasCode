/*
 * SPPValue.h
 *
 *  Created on: 17 Nov 2018
 *      Author: xasin
 */

#ifndef COMPONENTS_BT_SPP_SPPVALUE_H_
#define COMPONENTS_BT_SPP_SPPVALUE_H_

#include "stddef.h"
#include "stdint.h"

namespace Bluetooth {

class SPP_Server;

class SPP_Value {
protected:
	friend SPP_Server;

	SPP_Server& server;

	virtual void accept_data(const void *data, size_t length);
	virtual void on_reconnect();

public:
	const uint16_t id;

	SPP_Value(SPP_Server& server, uint16_t id);

	virtual bool write(const void *data, size_t length);
};

} /* namespace Bluetooth */

#endif /* COMPONENTS_BT_SPP_SPPVALUE_H_ */
