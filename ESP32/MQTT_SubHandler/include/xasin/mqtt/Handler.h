/*
 * Handler.h
 *
 *  Created on: 3 Feb 2019
 *      Author: xasin
 */

#ifndef ESP32_MQTT_SUBHANDLER_HANDLER_H_
#define ESP32_MQTT_SUBHANDLER_HANDLER_H_

#include "esp_event.h"

#include "mqtt_client.h"

#include <vector>
#include <string>
#include <functional>

namespace Xasin {
namespace MQTT {

extern const char *mqtt_tag;

typedef esp_mqtt_client_config_t mqtt_cfg;

struct MQTT_Packet {
	const std::string &topic;
	const std::string &data;
};
typedef std::function<void (const MQTT_Packet)> mqtt_callback;

class Subscription;

class Handler {
protected:
	friend Subscription;

	std::vector<Subscription *> subscriptions;

	esp_mqtt_client_handle_t mqtt_handle;

	bool wifi_connected;
	bool mqtt_connected;

public:
	Handler();

	void start(const mqtt_cfg &config);
	void start(const std::string URI);

	void wifi_handler(system_event_t *event);
	void mqtt_handler(esp_mqtt_event_t *event);

	void publish_to(const std::string &topic, void const *data, size_t length, bool retain = false, int qos = 0);
	void subscribe_to(const std::string &topic, mqtt_callback callback, int qos = 0);
};

} /* namespace MQTT */
} /* namespace Xasin */

#endif /* ESP32_MQTT_SUBHANDLER_HANDLER_H_ */