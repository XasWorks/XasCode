
#include "xnm/property_point/BaseOutput.h"
#include "xnm/property_point/BaseProperty.h"
#include "xnm/property_point/BaseHandler.h"

namespace XNM {
namespace PropertyPoint {

BaseProperty::BaseProperty(Handler &handler, const char *key) : 
	handler(handler),
	truth_holder(nullptr),
	change_index(handler.bump_change_index()), 
	
	key(key), readonly(false),
	initialized(true),
	on_update(nullptr) {
}

void BaseProperty::poke_update() {
	if(truth_holder != nullptr)
		return;

	change_index = handler.bump_change_index();
	initialized = true;

	cJSON * state_json = get_current_state();
	handler.broadcast_update(state_json, *this);

	cJSON_Delete(state_json);
}

cJSON * BaseProperty::get_current_state() {
	return cJSON_CreateNull();
}

void BaseProperty::set_json(const cJSON * data) {
	if(readonly)
		return;

	initialized = true;

	if(truth_holder == nullptr) {
		this->process_json_command(data);

		if(on_update) on_update();
	}
	else
		this->truth_holder->send_set_json(data, *this);
}

void BaseProperty::upd_json(const cJSON * upd_data) {
	if((truth_holder != nullptr) || (!initialized)) {
		process_json_command(upd_data);

		if(on_update) on_update();

		initialized = true;
	}
}

void BaseProperty::process_json_command(const cJSON * data) {}

BaseOutput * BaseProperty::get_truthholder() {
	return truth_holder;
}

void BaseProperty::init() {
	handler.insert_property(*this);
}

}
}