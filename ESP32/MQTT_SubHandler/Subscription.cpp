/*
 * Subscription.cpp
 *
 *  Created on: 3 Feb 2019
 *      Author: xasin
 */

#include "xasin/mqtt/Subscription.h"

#include "esp_log.h"

namespace Xasin {
namespace MQTT {

Subscription::Subscription(Handler &handler, const std::string topic, int qos)
	:mqtt_handler(handler),
	 topic(topic), qos(qos),
	 on_received(nullptr) {

	handler.subscriptions.push_back(this);

	if(handler.mqtt_connected)
		raw_subscribe();
}

Subscription::~Subscription() {
	puts("MQTT Unsubscribing!");

	bool conflictFound = false;
	for(auto s = mqtt_handler.subscriptions.begin(); s < mqtt_handler.subscriptions.end(); s++) {
		if((*s) == this)
			mqtt_handler.subscriptions.erase(s);
		else if((*s)->topic == this->topic)
			conflictFound = true;
	}

	if(conflictFound)
		return;

	if(mqtt_handler.mqtt_connected)
		esp_mqtt_client_unsubscribe(mqtt_handler.mqtt_handle, topic.data());
}

void Subscription::raw_subscribe() {
	ESP_LOGI(mqtt_tag, "Subscribing to %s", topic.data());
	esp_mqtt_client_subscribe(mqtt_handler.mqtt_handle, topic.data(), qos);
}

void Subscription::feed_data(MQTT_Packet data) {
	if(on_received == nullptr) return;
	if(data.topic.length() < topic.length()) return;

	std::string topicRest;

	int i=0;
	while(true) {
		if(topic.at(i) == '#') {
			topicRest = std::string(data.topic.data() + i, data.topic.length() - i);
			break;
		}
		if(topic.at(i) != data.topic.at(i)) return;

		i++;

		if(i == topic.length()) {
			if(i != data.topic.length())
				return;
			else
				break;
		}
	}

	ESP_LOGV(mqtt_tag, "Topic %s matched! (Topic-Rest:%s)", topic.data(), topicRest.data());
	on_received({topicRest, data.data});
}

} /* namespace MQTT */
} /* namespace Xasin */
