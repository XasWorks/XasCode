
#include "include/xnm/property_point/CustomProperty.h"

namespace XNM {
namespace PropertyPoint {

CustomProperty::CustomProperty(Handler &handler, const char *key) 
	: BaseProperty(handler, key),
		on_process(nullptr), on_get_state(nullptr) {
}

void CustomProperty::process_json_command(const cJSON * data) {
	if(on_process) on_process(data);
}
cJSON * CustomProperty::get_current_state() {
	cJSON * out = nullptr;
	
	if(on_get_state)
		out = on_get_state();

	if(out == nullptr)
		out = cJSON_CreateNull();

	return out;
}

}
}