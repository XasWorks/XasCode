/*
 * Endpoint.h
 *
 *  Created on: 1 Aug 2019
 *      Author: xasin
 */

#ifndef ESP32_SMOLHTTP_ENDPOINT_H_
#define ESP32_SMOLHTTP_ENDPOINT_H_

#include "esp_http_server.h"

#include <vector>
#include <string>

#include <functional>

#include "cJSON.h"

namespace Xasin {
namespace HTTP {

class Server;

class Endpoint {
protected:
	friend Server;

	Server &server;

	const std::string URI;

	httpd_req_t *req;

	void register_endpoint();
	void unregister_endpoint();

	esp_err_t nom_request(httpd_req_t *req);

public:
	static esp_err_t call_endpoint_func(httpd_req_t *req);

	Endpoint(Server &server, std::string URI);
	virtual ~Endpoint();

	void reply_string(const char *location);
	void set_type(const char *type);

	void reply_cJSON(const cJSON *item);

	std::function<void(httpd_req_t *req)> on_request;
};

} /* namespace HTTP */
} /* namespace Xasin */

#endif /* ESP32_SMOLHTTP_ENDPOINT_H_ */
