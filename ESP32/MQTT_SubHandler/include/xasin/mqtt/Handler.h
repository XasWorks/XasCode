/*
 * Handler.h
 *
 *  Created on: 3 Feb 2019
 *      Author: xasin
 */

#ifndef ESP32_MQTT_SUBHANDLER_HANDLER_H_
#define ESP32_MQTT_SUBHANDLER_HANDLER_H_

#include "esp_event.h"
#include "esp_wifi.h"

#include "mqtt_client.h"

#include <freertos/semphr.h>

#include <vector>
#include <string>
#include <functional>

#include <stdint.h>

namespace Xasin {
namespace MQTT {

enum wifif_ps_t {
	NONE = -1,
	DEFAULT = 0,
	MAX_MODEMSLEEP = 1,
	SKIP_STA = 2
};

extern const char *mqtt_tag;

typedef esp_mqtt_client_config_t mqtt_cfg;

struct MQTT_Packet {
	const std::string &topic;
	const std::string &data;
};
typedef MQTT_Packet Packet;

typedef std::function<void (const MQTT_Packet)> mqtt_callback;

class Subscription;

class Handler {
private:
	SemaphoreHandle_t subscription_config_lock;

protected:
	friend Subscription;

	std::vector<Subscription *> subscriptions;
	bool sub_sem_lock();
	void sub_sem_unlock();

	esp_mqtt_client_handle_t mqtt_handle;

	bool wifi_connected;
	bool mqtt_started;
	bool mqtt_connected;

	std::string status_topic;
	std::string status_msg;

	std::string base_topic;

public:
	static void set_nvs_uri(const char *new_uri);

	Handler();
	Handler(const std::string & base_topic);

	void topicsize_string(std::string &topic);

	void start(const mqtt_cfg &config);
	void start(const std::string URI);

	bool start_from_nvs();

	void stop();

	void wifi_handler(system_event_t *event);
	void mqtt_handler(esp_mqtt_event_t *event);

	void set_status(const std::string &newStatus);

	void publish_to(std::string topic, void const *data, size_t length, bool retain = false, int qos = 0);
	void publish_int(const std::string &topic, int32_t data, bool retain = false, int qos = 0);

	Subscription * subscribe_to(std::string topic, mqtt_callback callback, int qos = 1);

	uint8_t is_disconnected();
};

} /* namespace MQTT */
} /* namespace Xasin */

#endif /* ESP32_MQTT_SUBHANDLER_HANDLER_H_ */
