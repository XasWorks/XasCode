
#include "include/xnm/property_point/SingleProperty.h"

#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG
#include <esp_log.h>

namespace XNM {
namespace PropertyPoint {

#define SinglePropertySpecializationCode(type, json_get, json_test, json_set)\
template<> 								\
cJSON * SingleProperty<type>::get_current_state() { \
	cJSON * out = cJSON_CreateObject(); \
	cJSON_AddItemToObjectCS(out, "value", json_set); \
	return out; 						\
} 											\
template <> 							\
void SingleProperty<type>::process_json_command(const cJSON * data) { \
	ESP_LOGD("PROPP", "Property %s got update.", this->key); \
	if(!cJSON_IsObject(data)) 		\
		return; 							\
	data = cJSON_GetObjectItem(data, "value"); \
	if(!json_test) 					\
		return;							\
	auto json_val = json_get; 		\
	if(json_val == this->value) 	\
		return; 							\
	this->value = json_val;			\
	poke_update();						\
}


SinglePropertySpecializationCode(double, 
	data->valuedouble, 
	cJSON_IsNumber(data), 
	cJSON_CreateNumber(this->value))

SinglePropertySpecializationCode(int, 
	data->valueint, 
	cJSON_IsNumber(data), 
	cJSON_CreateNumber(this->value))

SinglePropertySpecializationCode(std::string, 
	std::string(data->valuestring), 
	cJSON_IsString(data), 
	cJSON_CreateStringReference(this->value.data()))

template<>
cJSON * SingleProperty<XNM::Neo::Color>::get_current_state() {
	cJSON * out = cJSON_CreateObject();
	cJSON_AddItemToObjectCS(out, "value", cJSON_CreateString(this->value.to_s().data()));
	return out;
}
template <>
void SingleProperty<XNM::Neo::Color>::process_json_command(const cJSON * data) {
	ESP_LOGD("PROPP", "Property %s got update.", this->key);
	if(!cJSON_IsObject(data))
		return;

	data = cJSON_GetObjectItem(data, "value");

	if(!cJSON_IsString(data))
		return;

	bool ok = false;
	auto c = XNM::Neo::Color::strtoc(data->valuestring, &ok);

	if(!ok)
		return;

	if(c == this->value)
			return;

	this->value = c;

	poke_update();
}

}
}