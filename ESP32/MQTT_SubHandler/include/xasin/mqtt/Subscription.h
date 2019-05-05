/*
 * Subscription.h
 *
 *  Created on: 3 Feb 2019
 *      Author: xasin
 */

#ifndef ESP32_MQTT_SUBHANDLER_SUBSCRIPTION_H_
#define ESP32_MQTT_SUBHANDLER_SUBSCRIPTION_H_

#include <string>
#include <functional>

#include "Handler.h"

namespace Xasin {
namespace MQTT {

class Subscription {
public:
	Handler &mqtt_handler;
protected:
	friend Handler;

	void raw_subscribe();
	void feed_data(MQTT_Packet data);

public:
	const std::string topic;
	const int qos;

	std::function<void(MQTT_Packet data)> on_received;

	Subscription(Handler &handler, const std::string topic, int qos = 0);
	~Subscription();
};

} /* namespace MQTT */
} /* namespace Xasin */

#endif /* ESP32_MQTT_SUBHANDLER_SUBSCRIPTION_H_ */
