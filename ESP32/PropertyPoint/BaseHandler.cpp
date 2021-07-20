
#include "xnm/property_point/BaseHandler.h"
#include "xnm/property_point/BaseOutput.h"
#include "xnm/property_point/BaseProperty.h"

#include <cstring>

#include <esp_log.h>

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

BaseProperty * Handler::operator[](const char * key) {
	for(auto p : properties) {
		if(strcmp(p->key, key) == 0)
			return p;
	}

	return nullptr;
}

}
}