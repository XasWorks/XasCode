/*
 * MQTTSlaveChannel.cpp
 *
 *  Created on: 4 Feb 2019
 *      Author: xasin
 */

#include "xasin/mqtt/MQTTSlaveChannel.h"

#include <cstdlib>
#include <cstring>

#include "esp_log.h"

namespace Xasin {
namespace MQTT {

using namespace Communication;

MQTTSlaveChannel::MQTTSlaveChannel(Handler &handler, const std::string baseTopic, RegisterBlock &mainRegister)
	: 	SlaveChannel(mainRegister),
		mqtt_handler(handler),
		subscription(handler, baseTopic + "/Write/#"),
		baseTopic(baseTopic) {

	subscription.on_received = [this](MQTT_Packet data) {
		uint16_t ID = strtol(data.topic.c_str(), nullptr, 16);

		ESP_LOGD("XAQTT:IDCOMM", "Got data for register: %d", ID);
		this->mainRegister.write_register(ID, {data.data.length(), data.data.data()});
	};

}

void MQTTSlaveChannel::send_update(uint16_t ID, Data_Packet data, bool retained) {
	char buffer[100];

	sprintf(buffer, "%s/Update/%02X", baseTopic.c_str(), ID);
	mqtt_handler.publish_to(buffer, data.data, data.length, retained, retained ? 0 : 1);
}

} /* namespace MQTT */
} /* namespace Xasin */
