

#pragma once

#include "BaseProperty.h"

#include <functional>

namespace XNM {
namespace PropertyPoint {

class CustomProperty : public BaseProperty {
public:
	CustomProperty(Handler &handler, const char *key);

	void process_json_command(const cJSON * data);
	cJSON * get_current_state();

	std::function<void (const cJSON *)> on_process;
	std::function<cJSON * (void)> on_get_state;

};

}
}