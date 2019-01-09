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

namespace Xasin {
namespace Communication {

class BasePipe {
public:
	BasePipe();

	std::vector<std::function<void (bool)>> on_connection_change;
	std::vector<std::function<void (unsigned short id, const void *data, size_t length)>> on_received;

	virtual bool is_connected();
	virtual bool send_packet(unsigned short id, const void *data, size_t length);

};

} /* namespace Communication */
} /* namespace Xasin */

#endif /* XASLIBS_ID_COMS_BASEPIPE_H_ */
