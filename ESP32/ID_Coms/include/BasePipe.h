/*
 * BasePipe.h
 *
 *  Created on: 9 Jan 2019
 *      Author: xasin
 */

#ifndef XASLIBS_ID_COMS_BASEPIPE_H_
#define XASLIBS_ID_COMS_BASEPIPE_H_

#include <cstring>
#include <functional>
#include <vector>
#include <map>

namespace Xasin {
namespace Communication {

// The virtual base class for Xasin-communication.
// It represents the endpoint of a connection between two devices.
// Communication itself is done by sending ID'd packets to "registers" in the other device.
// Additionally, read requests can be issued to fetch a given register from the other device.

// This way, this kind of communication can be used both for Master<->Master (both sending packets),
// And Master<->Slave (Master writing packets and issuing read requests)

// Thusly, each register must contain a callback function upon data receival,
// As well as a data pointer to be read from (optional)

struct Data_Packet {
	size_t length;
	void * const data;
};

class BasePipe {
public:
	BasePipe();

	std::vector<std::function<void (bool)>> on_connection_change;
	std::map<uint16_t, std::function<void (Data_Packet data)>> on_received;

	virtual bool is_connected();
	virtual bool send_packet(int id, const void *data, size_t length);
};

} /* namespace Communication */
} /* namespace Xasin */

#endif /* XASLIBS_ID_COMS_BASEPIPE_H_ */
