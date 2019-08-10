/*
 * Server.cpp
 *
 *  Created on: 1 Aug 2019
 *      Author: xasin
 */

#include "xasin/smolhttp/Endpoint.h"
#include "xasin/smolhttp/Server.h"

namespace Xasin {
namespace HTTP {

Server::Server() : server(nullptr), endpoints() {
}

Server::~Server() {
	stop();
}

void Server::stop() {
	if(server) {
		for(auto e : endpoints)
			e->unregister_endpoint();

		httpd_stop(server);
	}
}

void Server::start() {
	if(server)
		return;

	httpd_config_t config = HTTPD_DEFAULT_CONFIG();
	httpd_start(&server, &config);

	for(auto e : endpoints)
		e->register_endpoint();
}

} /* namespace HTTP */
} /* namespace Xasin */
