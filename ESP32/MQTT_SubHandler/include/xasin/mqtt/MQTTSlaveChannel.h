/*
 * MQTTSlaveChannel.h
 *
 *  Created on: 4 Feb 2019
 *      Author: xasin
 */

#ifndef ESP32_MQTT_SUBHANDLER_MQTTSLAVECHANNEL_H_
#define ESP32_MQTT_SUBHANDLER_MQTTSLAVECHANNEL_H_

#include "RegisterBlock.h"

#include "Handler.h"
#include "Subscription.h"

namespace Xasin {
namespace MQTT {


class MQTTSlaveChannel: public Communication::SlaveChannel {
private:
	Handler &mqtt_handler;
	Subscription subscription;

	const std::string baseTopic;

public:
	MQTTSlaveChannel(Handler &handler, const std::string topic, Communication::RegisterBlock &mainRegister);

	void send_update(uint16_t ID, const Communication::Data_Packet data, bool retained = false);
};

} /* namespace MQTT */
} /* namespace Xasin */

#endif /* ESP32_MQTT_SUBHANDLER_MQTTSLAVECHANNEL_H_ */
