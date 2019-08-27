/*
 * Client.cpp
 *
 *  Created on: 13 Aug 2019
 *      Author: xasin
 */

#include "xasin/smolhttp/Client.h"

#include <algorithm>
#include <cstring>

namespace Xasin {
namespace HTTP {

Client::Client(std::string baseURL) :
		client(nullptr), note_task(nullptr),
		base_url(baseURL),
		data_buffer() {

	esp_http_client_config_t cliCFG = {};

	cliCFG.url = base_url.data();
	cliCFG.user_data = this;
	cliCFG.event_handler = Client::call_client_handler;

	client = esp_http_client_init(&cliCFG);
}

Client::~Client() {
	if(client != nullptr)
		esp_http_client_cleanup(client);
}

esp_err_t Client::call_client_handler(esp_http_client_event_t *evt) {
	if(evt->user_data) {
		reinterpret_cast<Client*>(evt->user_data)->handle_event(evt);
	}

	return ESP_OK;
}

void Client::handle_event(esp_http_client_event_t *evt) {
	if(evt->data_len > (10*1024))
		return;

//	data_buffer.clear();
//	data_buffer.reserve(evt->data_len);
//
//	const uint8_t *data_ptr = reinterpret_cast<const uint8_t*>(evt->data);
//	std::copy_n(data_ptr, evt->data_len, std::back_inserter(data_buffer));
//
//	if(note_task)
//		xTaskNotify(note_task, 0, eNoAction);
}

void Client::send_cJSON(std::string endpoint, const cJSON *obj) {
	std::string combined_url = base_url + endpoint;
	esp_http_client_set_url(client, combined_url.data());
	esp_http_client_set_method(client, HTTP_METHOD_POST);

	char *json_str = cJSON_PrintUnformatted(obj);
	esp_http_client_set_post_field(client, json_str, strlen(json_str));

	esp_http_client_perform(client);

	delete json_str;
}

cJSON *Client::get_cJSON(std::string endpoint) {
	esp_http_client_set_method(client, HTTP_METHOD_GET);

	std::string combined_url = base_url + endpoint;
	esp_http_client_set_url(client, combined_url.data());

	esp_http_client_set_post_field(client, nullptr, 0);

	esp_http_client_perform(client);

	if(esp_http_client_get_content_length(client) <= 0)
		return cJSON_CreateNull();

	std::vector<char> jsonBuffer(esp_http_client_get_content_length(client));

	esp_http_client_read(client, jsonBuffer.data(), jsonBuffer.size());

	return cJSON_Parse(jsonBuffer.data());
}

} /* namespace HTTP */
} /* namespace Xasin */
