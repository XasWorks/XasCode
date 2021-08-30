
#include "xnm/property_point/BaseHandler.h"
#include "xnm/property_point/BaseOutput.h"
#include "xnm/property_point/BaseProperty.h"

#include <cstring>

#define LOG_LOCAL_LEVEL ESP_LOG_INFO
#include <esp_log.h>

#define TAG "propp-handler"

namespace XNM {
namespace PropertyPoint {

Handler::Handler() 
	: outputs(), properties(),
	change_index(0) {
}

void Handler::insert_property(BaseProperty &prop) {
	if(prop.key == nullptr)
		return;
	
	for(auto p : properties) {
		if(p->key == prop.key)
			ESP_LOGE("property_handler", "A property with key %s was inserted twice!", prop.key);
	}

	properties.push_back(&prop);
}
void Handler::insert_output(BaseOutput &output) {
	outputs.push_back(&output);
}

void Handler::broadcast_update(cJSON * item, BaseProperty &prop) {
	for(auto output : outputs) {
		if(output == prop.truth_holder)
			continue;

		output->send_upd_json(item, prop);
	}
}

uint32_t Handler::bump_change_index() {
	return change_index++;
}
uint32_t Handler::get_change_index() {
	return change_index;
}

void Handler::process_command_json(const char * str, BaseOutput &output) {
	cJSON * data = cJSON_Parse(str);

	ESP_LOGD(TAG, "Command JSON got string '%s', %s", str, cJSON_IsObject(data) ? "parsed" : "did not parse!");

	if(!cJSON_IsObject(data))
		return;

	cJSON * set_data = cJSON_GetObjectItem(data, "set");
	if(cJSON_IsObject(set_data)) {
		cJSON * i = nullptr;

		cJSON_ArrayForEach(i, set_data) {
			ESP_LOGD(TAG, "Processing command for key '%s'", i->string);

			BaseProperty * ptr = (*this)[i->string];

			if(!ptr) {
				ESP_LOGD(TAG, "Did not find %s", i->string);
				continue;
			}

			ptr->set_json(i);
		}
	}

	if(cJSON_IsTrue(cJSON_GetObjectItem(data, "get_all"))) {
		ESP_LOGD(TAG, "Pushing all properties to BLE!");
		
		for(auto p : properties) {
			cJSON * update_json = p->get_current_state();
			output.send_upd_json(update_json, *p);

			cJSON_Delete(update_json);
		}
	}

	cJSON_Delete(data);
}

BaseProperty * Handler::operator[](const char * key) {
	for(auto p : properties) {
		if(strcmp(p->key, key) == 0)
			return p;
	}

	return nullptr;
}

}
}