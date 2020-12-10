/*
 * UDP.h
 *
 *  Created on: 15 Aug 2019
 *      Author: xasin
 */

#ifndef ESP32_RAWSOCKS_UDP_H_
#define ESP32_RAWSOCKS_UDP_H_

#include <string>
#include <vector>

#include "sock_target.h"

#include "esp_netif.h"
#include "lwip/sockets.h"

namespace Xasin {
namespace Socks {

class UDP {
private:
	int socket_no;

	sockaddr_in target_sock_addr;

public:
	const std::string tgt_addr;
	const int port;

	socklen_t   last_received_from_len;
	sockaddr_in last_received_from_sockaddr;

	sock_target_t last_received_from;

	UDP(std::string tgt_addr, int port);
	virtual ~UDP();

	void open_remote();
	void open_local();

	void send(const void *data, size_t data_len, sock_target_t target = {});
	int  wait_on_receive(void *bufLoc, size_t bufferLength);
};

} /* namespace Socks */
} /* namespace Xasin */

#endif /* ESP32_RAWSOCKS_UDP_H_ */
