
#include "include/xnm/property_point/BLEOutput.h"

namespace XNM {
namespace PropertyPoint {

BLEOutput::BLEOutput(Handler &handler, BLE::Server &ble_server) 
	: 
	BaseOutput(handler), ble(ble_server, ble_srv_uuid, ble_chr_uuid) {

}

void BLEOutput::send_upd_json(const cJSON * item, BaseProperty &prop) {
	cJSON * out_obj = cJSON_CreateObject();

	cJSON * upd_obj = cJSON_CreateObject();
	cJSON_AddItemToObjectCS(out_obj, "upd", upd_obj);

	cJSON_AddItemReferenceToObject(upd_obj, prop.key, const_cast<cJSON*>(item));

	char * out = cJSON_Print(out_obj);

	ble.send_str(out);

	cJSON_free(out);
	cJSON_Delete(out_obj);
}

void BLEOutput::init() {
	ble.init();

	ble.on_receive = [this](std::vector<char> && data) {
		this->handler.process_command_json(data.data(), *this);
	};
}

}
}