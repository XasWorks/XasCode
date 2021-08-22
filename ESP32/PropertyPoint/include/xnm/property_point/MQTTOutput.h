
#pragma once

#include "BaseHandler.h"
#include "BaseOutput.h"
#include "BaseProperty.h"

#include <xasin/mqtt.h>

namespace XNM {
namespace PropertyPoint {

typedef ::Xasin::MQTT::Handler MQTT;

class MQTTOutput : public BaseOutput {
private:
	MQTT &mqtt;

protected:
	void send_upd_json(const cJSON * item, BaseProperty &prop);
	void send_set_json(const cJSON * item, BaseProperty &prop);

public:
	MQTTOutput(Handler &handler, MQTT &mqtt);

	void init();
};

}
}