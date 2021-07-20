
#pragma once

#include <stdint.h>
#include <cJSON.h>

#include <vector>
#include <map>

namespace XNM {
namespace PropertyPoint {

class BaseOutput;
class BaseProperty;

class Handler {
private:
	std::vector<BaseOutput *>   outputs;
	std::vector<BaseProperty *> properties;

	uint32_t change_index;

protected:
friend BaseProperty;
friend BaseOutput;

	void insert_property(BaseProperty &prop);
	void insert_output(BaseOutput &output);

	const char * config_json;

	/*	Broadcast the current state of a property
	 *
	 * This will broadcast the updated cJSON structure to all outputs
	 * that are *not* the truthholder of the item itself (as we will have received the update from there).
	 *
	 * Does not delete the cJSON item after completing!
	 */
	void broadcast_update(cJSON * item, BaseProperty &prop);

public:
	Handler();

	uint32_t bump_change_index();
	uint32_t get_change_index();

	BaseProperty *operator[](const char *key);
};

}
}