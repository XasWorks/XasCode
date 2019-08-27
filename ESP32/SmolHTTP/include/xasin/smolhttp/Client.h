/*
 * Client.h
 *
 *  Created on: 13 Aug 2019
 *      Author: xasin
 */

#ifndef ESP32_SMOLHTTP_CLIENT_H_
#define ESP32_SMOLHTTP_CLIENT_H_

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_http_client.h"

#include <vector>
#include <string>

#include "cJSON.h"

namespace Xasin {
namespace HTTP {

class Client {
private:
	void handle_event(esp_http_client_event_t *evt);

	esp_http_client_handle_t client;

	TaskHandle_t note_task;

public:
	const std::string base_url;
	std::vector<uint8_t> data_buffer;

	static esp_err_t call_client_handler(esp_http_client_event_t *evt);

	Client(std::string base_url);
	virtual ~Client();

	void send_cJSON(std::string endpoint, const cJSON *obj);
	cJSON *get_cJSON(std::string endpoint);

};

} /* namespace HTTP */
} /* namespace Xasin */

#endif /* ESP32_SMOLHTTP_CLIENT_H_ */
