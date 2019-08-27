/*
 * HTTPPropChannel.cpp
 *
 *  Created on: 4 Aug 2019
 *      Author: xasin
 */

#include "xasin/smolhttp/HTTPPropChannel.h"
#include "xasin/propertypoint/PropertyHandler.h"
#include "xasin/propertypoint/BaseProperty.h"


#define LOG_LOCAL_LEVEL ESP_LOG_INFO
#include "esp_log.h"

namespace Xasin {
namespace HTTP {

std::array<char, 1024> HTTPPropBuffer = {};

HTTPPropChannel::HTTPPropChannel(Server &server, std::string uri, PropP::PropertyHandler &handler)
	: BaseChannel(handler),
	  property_endpoint(server, uri) {

	property_endpoint.on_request = [this](httpd_req_t *req) {
		if(req->method == HTTP_GET) {
			uint32_t modified_since = 0;

			char query_buffer[30] = {};
			auto ret = httpd_req_get_url_query_str(req, query_buffer, 30);

			if(ret == ESP_OK) {
				char mod_no_buffer[10] = {};
				ret = httpd_query_key_value(query_buffer, "lastRev", mod_no_buffer, 10);
				if(ret == ESP_OK)
					modified_since = atol(mod_no_buffer);
			}

			cJSON *data_obj = this->handler.get_cJSON_since(modified_since);

			this->property_endpoint.reply_cJSON(data_obj);
			cJSON_Delete(data_obj);
		}

		else if(req->method == HTTP_POST) {
			if(req->content_len > HTTPPropBuffer.size())
				return;

			size_t len = httpd_req_recv(req, HTTPPropBuffer.data(), req->content_len);
			HTTPPropBuffer[len] = '\0';

			cJSON *inputObj = cJSON_Parse(HTTPPropBuffer.data());
			if(!cJSON_IsObject(inputObj)) {
				this->property_endpoint.reply_string("Invalid JSON");
				cJSON_Delete(inputObj);
				return;
			}

			this->handler.feed_cJSON(inputObj);

			this->property_endpoint.reply_string("OK");
			cJSON_Delete(inputObj);
		}
	};
}

HTTPPropChannel::~HTTPPropChannel() {
}

} /* namespace HTTP */
} /* namespace Xasin */
