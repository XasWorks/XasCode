/*
 * Server.h
 *
 *  Created on: 1 Aug 2019
 *      Author: xasin
 */

#ifndef ESP32_SMOLHTTP_SERVER_H_
#define ESP32_SMOLHTTP_SERVER_H_

#include "esp_http_server.h"

#include <vector>

namespace Xasin {
namespace HTTP {

class Endpoint;

class Server {
protected:
	friend Endpoint;

	httpd_handle_t server;

	std::vector<Endpoint*> endpoints;

public:
	Server();
	~Server();

	void stop();
	void start();
};

} /* namespace HTTP */
} /* namespace Xasin */

#endif /* ESP32_SMOLHTTP_SERVER_H_ */
