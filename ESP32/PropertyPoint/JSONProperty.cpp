
#include "include/xnm/property_point/JSONProperty.h"

#include <cstring>

namespace XNM {
namespace PropertyPoint {

JSONObjProperty::JSONObjProperty(Handler &handler, const char *key) 
	: BaseProperty(handler, key), merge_json(true) {

		obj = cJSON_CreateObject();
}

JSONObjProperty::~JSONObjProperty() {
	cJSON_Delete(obj);
}

cJSON * JSONObjProperty::fetch_key(const char *key) {
	cJSON * out = cJSON_GetObjectItem(obj, key);

	if(out == nullptr)
		out = cJSON_AddNullToObject(obj, key);

	return out;
}
cJSON * JSONObjProperty::operator[](const char *key) {
	return cJSON_GetObjectItem(obj, key);
}

void JSONObjProperty::delete_key(const char *key) {
	cJSON_DeleteItemFromObject(obj, key);
}

void JSONObjProperty::process_json_command(const cJSON * data) {
	if(merge_json) {
		cJSON * item;
		cJSON_ArrayForEach(item, data) {
			item = cJSON_Duplicate(item, true);

			cJSON_ReplaceItemViaPointer(obj, fetch_key(item->string), item);
		}
	}
	else {
		cJSON_Delete(obj);
		obj = cJSON_Duplicate(data, true);
	}
}

cJSON * JSONObjProperty::get_current_state() {
	cJSON * out = reinterpret_cast<cJSON*>(cJSON_malloc(sizeof(cJSON)));

	memcpy(out, obj, sizeof(cJSON));
	out->type |= cJSON_IsReference;
	out->next = out->prev = nullptr;

	return out;
}

double JSONObjProperty::get_num(const char *key) {
	auto item = cJSON_GetObjectItem(obj, key);
	if(!cJSON_IsNumber(item))
		return 0;

	return item->valuedouble;
}

void JSONObjProperty::set_num(double value, const char *key) {
	auto item = fetch_key(key);

	item->type = (item->type & (cJSON_IsReference | cJSON_StringIsConst)) | cJSON_Number;
	cJSON_SetNumberValue(item, value);
}

void JSONObjProperty::update_done() {
	poke_update();
}

}
}