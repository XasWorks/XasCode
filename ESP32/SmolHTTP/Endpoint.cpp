/*
 * Endpoint.cpp
 *
 *  Created on: 1 Aug 2019
 *      Author: xasin
 */

#include "xasin/smolhttp/Endpoint.h"
#include "xasin/smolhttp/Server.h"

namespace Xasin {
namespace HTTP {

esp_err_t Endpoint::call_endpoint_func(httpd_req_t *req) {
	auto endpoint = reinterpret_cast<Endpoint*>(req->user_ctx);

	return endpoint->nom_request(req);
}

Endpoint::Endpoint(Server &server, std::string URI) :
		server(server),
		URI(URI),
		req(nullptr),
		on_request(nullptr) {

	server.endpoints.push_back(this);
}

Endpoint::~Endpoint() {
	unregister_endpoint();
	// TODO Handle deletion from server list!
}

void Endpoint::register_endpoint() {
	httpd_uri_t uri_cfg = {
			.uri 	= URI.data(),
			.method	= HTTP_GET,
			.handler = call_endpoint_func,
			.user_ctx = this,
	};
	httpd_register_uri_handler(server.server, &uri_cfg);

	uri_cfg.method = HTTP_POST;
	httpd_register_uri_handler(server.server, &uri_cfg);
}

void Endpoint::unregister_endpoint() {
}

esp_err_t Endpoint::nom_request(httpd_req_t *req) {
	this->req = req;

	if(on_request)
		on_request(req);

	return ESP_OK;
	this->req = nullptr;
}

void Endpoint::reply_string(const char *location) {
	if(req) {
		httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");

		httpd_resp_send(req, location, strlen(location));
	}
}
void Endpoint::set_type(const char *type) {
	if(req)
		httpd_resp_set_type(req, type);
}
void Endpoint::reply_cJSON(const cJSON *item) {
	set_type(HTTPD_TYPE_JSON);

	char *str = cJSON_PrintUnformatted(item);
	reply_string(str);

	delete str;
}

} /* namespace HTTP */
} /* namespace Xasin */
