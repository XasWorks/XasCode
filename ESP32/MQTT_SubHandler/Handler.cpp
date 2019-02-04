/*
 * Handler.cpp
 *
 *  Created on: 3 Feb 2019
 *      Author: xasin
 */

#include "xasin/mqtt/Handler.h"
#include "xasin/mqtt/Subscription.h"

namespace Xasin {
namespace MQTT {

esp_err_t mqtt_handle_caller(esp_mqtt_event_t *event) {
	reinterpret_cast<Handler*>(event->user_context)->mqtt_handler(event);

	return ESP_OK;
}

Handler::Handler()
	: subscriptions(),
	  mqtt_handle(nullptr),
	  wifi_connected(false), mqtt_connected(false) {
}

void Handler::start(const mqtt_cfg &config) {
	mqtt_cfg modConfig = config;

	modConfig.event_handle = mqtt_handle_caller;
	modConfig.user_context = this;

	mqtt_handle = esp_mqtt_client_init(&modConfig);
	if(wifi_connected)
		esp_mqtt_client_start(mqtt_handle);
}
void Handler::start(const std::string uri) {
	mqtt_cfg config = {};
	config.uri = uri.data();

	start(config);
}

void Handler::wifi_handler(system_event_t *event) {
	switch(event->event_id) {
	case SYSTEM_EVENT_STA_GOT_IP:
	case SYSTEM_EVENT_ETH_GOT_IP:
		puts("Wifi connected");

		wifi_connected = true;
		esp_mqtt_client_start(mqtt_handle);
	break;

	case SYSTEM_EVENT_STA_LOST_IP:
		wifi_connected = false;
		mqtt_connected = false;
		esp_mqtt_client_stop(mqtt_handle);
	break;

	default: break;
	}
}

void Handler::mqtt_handler(esp_mqtt_event_t *event) {
	puts("MQTT got event!");

	switch(event->event_id) {
	case MQTT_EVENT_CONNECTED:
		puts("MQTT Connected!");

		mqtt_connected = true;

		if(!event->session_present) {
			for(auto s : subscriptions)
				s->raw_subscribe();
		}
	break;

	case MQTT_EVENT_DISCONNECTED:
		mqtt_connected = false;
	break;

	case MQTT_EVENT_DATA: {
		const std::string dataString = std::string(event->data, event->data_len);
		const std::string topicString = std::string(event->topic, event->topic_len);

		for(auto s : subscriptions)
			s->feed_data({topicString, dataString});
	}
	break;

	default: break;
	}
}

void Handler::publish_to(const std::string &topic, const void *data, size_t length, bool retain, int qos) {
	if(!mqtt_connected)
		return;

	esp_mqtt_client_publish(mqtt_handle, topic.data(), reinterpret_cast<const char*>(data)
				, length, qos, retain);
}

} /* namespace MQTT */
} /* namespace Xasin */
