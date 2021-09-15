/*
 * Handler.cpp
 *
 *  Created on: 3 Feb 2019
 *      Author: xasin
 */

#include "xasin/mqtt/Handler.h"
#include "xasin/mqtt/Subscription.h"

#include "esp_wpa2.h"

#include "nvs.h"
#include "nvs_flash.h"

#include <lwip/apps/sntp.h>

// #define LOG_LOCAL_LEVEL ESP_LOG_DEBUG
#include "esp_log.h"

#include <xnm/net_helpers.h>

namespace Xasin {
namespace MQTT {

const char *mqtt_tag = "XNMQTT";

esp_err_t mqtt_handle_caller(esp_mqtt_event_t *event) {
	reinterpret_cast<Handler*>(event->user_context)->mqtt_handler(event);

	return ESP_OK;
}

Handler::Handler()
	: subscriptions(),
	  mqtt_handle(nullptr),
	  wifi_connected(false),
	  mqtt_started(false), mqtt_connected(false),
	  status_topic("status"), status_msg("OK") {


	ESP_LOGD(mqtt_tag, "Early MQTT init");
	
	base_topic.reserve(64);

	base_topic 	= "/esp32/";
	base_topic += CONFIG_PROJECT_NAME;
	base_topic += "/";
	base_topic += XNM::NetHelpers::get_device_id();
	base_topic += "/";
}
Handler::Handler(const std::string &base_t) : Handler() {
	base_topic = base_t;
}

bool Handler::sub_sem_lock() {
	if(subscription_config_lock == nullptr)
		subscription_config_lock = xSemaphoreCreateRecursiveMutex();

	assert(xSemaphoreTakeRecursive(subscription_config_lock, 4000));

	ESP_LOGD(mqtt_tag, "Locked mutex!");
	return true;
}

void Handler::sub_sem_unlock() {
	assert(subscription_config_lock);

	assert(xSemaphoreGiveRecursive(subscription_config_lock));

	ESP_LOGD(mqtt_tag, "Unlocked mutex!");
}

void Handler::topicsize_string(std::string &topic) {
	if(topic[0] == '/')
		return;

	topic = base_topic + topic;
}

void Handler::start(const mqtt_cfg &config) {
	assert(!mqtt_started);

	mqtt_started = true;
	esp_log_level_set("MQTT_CLIENT", ESP_LOG_NONE);

	mqtt_cfg modConfig = config;

	modConfig.event_handle = mqtt_handle_caller;
	modConfig.user_context = this;

	mqtt_handle = esp_mqtt_client_init(&modConfig);

	if(wifi_connected)
		esp_mqtt_client_start(mqtt_handle);
	
	vTaskDelay(100/portTICK_PERIOD_MS);
}
void Handler::start(const std::string uri) {
	mqtt_cfg config = {};
	config.uri = uri.data();

	config.disable_clean_session = false;

	config.use_global_ca_store = true;

	topicsize_string(this->status_topic);

	config.lwt_topic = this->status_topic.data();
	config.lwt_retain = true;
	config.lwt_qos = 1;
	config.lwt_msg_len = 0;

	config.keepalive = 5;

	start(config);
}

bool Handler::start_from_nvs() {
	// if(!wifi_was_configured) {
	// 	if(!Handler::start_wifi_from_nvs())
	// 		return false;
	// }

	nvs_handle_t read_handle;

	char uri_buffer[254] = {};
	size_t uri_length = 254;

	nvs_open("xasin", NVS_READONLY, &read_handle);
	auto ret = nvs_get_str(read_handle, "mqtt_uri", uri_buffer, &uri_length);
	nvs_close(read_handle);

	if(ret != ESP_OK)
		return false;

	ESP_LOGI(mqtt_tag, "Starting from NVS with URI %s", uri_buffer);
	start(uri_buffer);

	return true;
}

void Handler::stop() {
	ESP_LOGE(mqtt_tag, "Please note to your local dragon operator that MQTT::stop is not implemented yet!");
}

void Handler::set_nvs_uri(const char *new_uri) {
	if(strlen(new_uri) > 500)
		return;

	nvs_handle_t write_handle;

	nvs_open("xasin", NVS_READWRITE, &write_handle);
	nvs_set_str(write_handle, "mqtt_uri", new_uri);

	nvs_commit(write_handle);
	nvs_close(write_handle);
}

void Handler::wifi_handler(system_event_t *event) {
	switch(event->event_id) {
	case SYSTEM_EVENT_STA_GOT_IP:
	case SYSTEM_EVENT_ETH_GOT_IP:
		wifi_connected = true;

		if(mqtt_started)
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
	switch(event->event_id) {
	case MQTT_EVENT_CONNECTED:
		mqtt_connected = true;

		ESP_LOGD(mqtt_tag, "Beginning subscription cycle...");

		if(!sub_sem_lock())
			return;

		if(!event->session_present) {
			for(auto s : subscriptions)
				s->raw_subscribe();
		}

		sub_sem_unlock();

		if(status_topic != "")
			this->publish_to(status_topic, status_msg.data(), status_msg.length(), true);
		ESP_LOGI(mqtt_tag, "Reconnected and subscribed");
	break;

	case MQTT_EVENT_DISCONNECTED:
		mqtt_connected = false;
		ESP_LOGW(mqtt_tag, "Disconnected from broker!");

//		if(wifi_connected)
//			esp_mqtt_client_reconnect(mqtt_handle);
	break;

	case MQTT_EVENT_DATA: {
		const std::string dataString = std::string(event->data, event->data_len);
		const std::string topicString = std::string(event->topic, event->topic_len);

		ESP_LOGV(mqtt_tag, "Data for topic %s:", topicString.data());
		ESP_LOG_BUFFER_HEXDUMP(mqtt_tag, dataString.data(), dataString.length(), ESP_LOG_VERBOSE);

		if(!sub_sem_lock())
			return;

		for(auto s : subscriptions)
			s->feed_data({topicString, dataString});
		
		sub_sem_unlock();
	}
	break;

	default: break;
	}

}

void Handler::set_status(const std::string &newStatus) {
	if(status_topic == "")
		return;

	status_msg = newStatus;
	if(mqtt_connected)
		this->publish_to(status_topic, status_msg.data(), status_msg.length(), true);
}

void Handler::publish_to(std::string topic, const void *data, size_t length, bool retain, int qos) {
	if(!mqtt_connected) {
		ESP_LOGD(mqtt_tag, "Packet to %s dropped (disconnected)", topic.data());
		return;
	}

	topicsize_string(topic);

	ESP_LOGV(mqtt_tag, "Publishing to %s", topic.data());
	ESP_LOG_BUFFER_HEXDUMP(mqtt_tag, data, length, ESP_LOG_VERBOSE);
	
	esp_mqtt_client_publish(mqtt_handle, topic.data(), reinterpret_cast<const char*>(data)
				, length, qos, retain);
}

void Handler::publish_int(const std::string &topic, int32_t data, bool retain, int qos) {
	char buffer[10] = {};
	sprintf(buffer, "%d", data);

	publish_to(topic, buffer, strlen(buffer), retain, qos);
}

Subscription * Handler::subscribe_to(std::string topic, mqtt_callback cb, int qos) {
	topicsize_string(topic);	

	// NO subscribing necessary here, the subscription class already handles this
	auto nSub = new Subscription(*this, topic, qos);
	nSub->on_received = cb;

	return nSub;
}

uint8_t Handler::is_disconnected() {
	if(!mqtt_started)
		return 255;

	if(mqtt_connected)
		return 0;
	if(wifi_connected)
		return 1;

	return 2;
}


} /* namespace MQTT */
} /* namespace Xasin */
