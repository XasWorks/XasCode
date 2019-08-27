/*
 * sock_target.h
 *
 *  Created on: 15 Aug 2019
 *      Author: xasin
 */

#ifndef ESP32_RAWSOCKS_SOCK_TARGET_H_
#define ESP32_RAWSOCKS_SOCK_TARGET_H_

#include <stdint.h>

namespace Xasin {
namespace Socks {

struct sock_target_t {
	uint32_t target_location;
	int 	 target_port;
};

}
}


#endif /* ESP32_RAWSOCKS_SOCK_TARGET_H_ */
