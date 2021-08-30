

#include "xnm/property_point/MQTTOutput.h"

#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG
#include <esp_log.h>

namespace XNM {
namespace PropertyPoint {

MQTTOutput::MQTTOutput(Handler &handler, MQTT &mqtt) : BaseOutput(handler),
	mqtt(mqtt) {
}

void MQTTOutput::init() {
		mqtt.subscribe_to("set/#", [this](Xasin::MQTT::Packet data) {
		auto prop = handler[data.topic.data()];


		if(prop == nullptr)
			return;
		if(this == prop->get_truthholder())
			return;

		auto json = cJSON_Parse(data.data.data());
		if(json == nullptr)
			return;


		if(cJSON_IsObject(json)) {
			ESP_LOGD("PROPP", "Got set for %s, ptr %p", data.topic.data(), prop);
			prop->set_json(json);
		}

		cJSON_Delete(json);
	});

	mqtt.subscribe_to("upd/#", [this](Xasin::MQTT::Packet data) {
		auto prop = handler[data.topic.data()];


		if(prop == nullptr)
			return;
			
		if((this != prop->get_truthholder()) && (prop->initialized))
			return;

		auto json = cJSON_Parse(data.data.data());
		if(json == nullptr)
			return;

		if(cJSON_IsObject(json)) {
			ESP_LOGD("PROPP", "Got upd for %s, ptr %p", data.topic.data(), prop);
			
			prop->upd_json(json);
		}

		cJSON_Delete(json);
	});
}

void MQTTOutput::send_upd_json(const cJSON * item, BaseProperty &prop) {
	char * json_msg = cJSON_Print(item);

	mqtt.publish_to(std::string("upd/") + prop.key, json_msg, strlen(json_msg), true);

	cJSON_free(json_msg);
}

void MQTTOutput::send_set_json(const cJSON * item, BaseProperty &prop) {
	char * json_msg = cJSON_Print(item);

	mqtt.publish_to(std::string("set/") + prop.key, json_msg, strlen(json_msg), true);

	cJSON_free(json_msg);
}

}
}