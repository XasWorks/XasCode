/*
 * UDP.cpp
 *
 *  Created on: 15 Aug 2019
 *      Author: xasin
 */

#include "xasin/socks/UDP.h"

#include "lwip/err.h"
#include "lwip/sys.h"

#include <lwip/netdb.h>

namespace Xasin {
namespace Socks {

UDP::UDP(std::string tgt_addr, int port)
	: 	socket_no(-1),
		tgt_addr(tgt_addr), port(port),
		last_received_from_len(), last_received_from_sockaddr(),
		last_received_from() {

	target_sock_addr = {};
	if(tgt_addr != "") {
		target_sock_addr.sin_addr.s_addr = inet_addr(tgt_addr.data());
		target_sock_addr.sin_family = AF_INET;
		target_sock_addr.sin_port   = htons(port);
	}
}

UDP::~UDP() {
	if(socket_no >= 0) {
		shutdown(socket_no, 0);
		close(socket_no);
	}
}

void UDP::open_remote() {
	socket_no = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
}
void UDP::open_local() {
	socket_no = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);

	sockaddr_in dest_addr = {};
	dest_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	dest_addr.sin_family = AF_INET;
	dest_addr.sin_port = htons(port);

	bind(socket_no, reinterpret_cast<sockaddr*>(&dest_addr), sizeof(dest_addr));
}

void UDP::send(const void *data, size_t length, sock_target_t target) {
	if(this->tgt_addr == "") {
		sockaddr_in target_addr = {};
		target_addr.sin_addr.s_addr = target.target_location;
		target_addr.sin_family = AF_INET;
		target_addr.sin_port = htons(target.target_port);

		sendto(socket_no, data, length, 0, reinterpret_cast<sockaddr *>(&target_addr), sizeof(target_addr));
	}
	else
		sendto(socket_no, data, length, 0, reinterpret_cast<sockaddr *>(&target_sock_addr), sizeof(target_sock_addr));
}

int UDP::wait_on_receive(void *buffer_location, size_t buffer_length) {
	int length = recvfrom(socket_no, buffer_location, buffer_length, 0, reinterpret_cast<sockaddr*>(&last_received_from_sockaddr), &last_received_from_len);

	last_received_from.target_location = last_received_from_sockaddr.sin_addr.s_addr;
	last_received_from.target_port	   = last_received_from_sockaddr.sin_port;

	return length;
}

} /* namespace Socks */
} /* namespace Xasin */
