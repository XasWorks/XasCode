
#pragma once

#include <cJSON.h>

#include <stdint.h>
#include <functional>

#include "PropertyState.h"

namespace XNM {
namespace PropertyPoint {

class Handler;
class BaseOutput;

class BaseProperty {
private:
	Handler &handler;

protected:
friend Handler;

	BaseOutput * truth_holder;

	uint32_t change_index;

	state_t property_state;

public:
	// Pointer to the key of this property.
	// Must be a constant char (presumably in program memory), and 
	// must be unique!
	const char * const key;

	bool readonly;
	// Whether or not this property is already initialized.
	// When set to false, will allow an output with persistence (presumably only the MQTT Output) to update this
	// property once, to initialize its state.
	bool initialized; 
	
	BaseProperty(Handler &handler, const char *key);

	BaseProperty & operator=(const BaseProperty &) = delete;
	BaseProperty(const BaseProperty&) = delete;

	// Call whenever this property is changed.
	//
	// This function call will initiate a update cycle for 
	// the corresponding BaseHandler, as well as nudging the 
	// change index of this property.
	void poke_update();

	std::function<void(void)> on_update;

	/* Return a cJSON_Delete-able cJSON item that describes the current
	 *	state of this property.
	 *
	 * How exactly the state is constructed does not matter. The default, 
	 * however, is {"_value":VALUE, "_state":STATE}
	 * 
	 * Note: The returned JSON *must* be deleted later!
	 */
	virtual cJSON * get_current_state();

	/* Feed a set command to this property
	 * 
	 * This will either:
	 *  - Pass the data structure to the "process_set_json" function,
	 * 		which the user must override, or it will
	 *	 - Forward the JSON structure towards the "truthholder" output when
	 *			this is set, in order to let another system handle it.
	 */
	void set_json(const cJSON * data);
	
	/*	Feed an update JSON structure to this property
	 *
	 *	This will either:
	 *	 - Ignore the update command if this property is its own
	 *			truthholder (i.e. the truthholder is nullptr), or
	 *	 - Will forward it to the user's "process_upd_json" function to
	 *			parse and accept the new json. Afterwards it will trigger
	 *			poke_update() in order to forward the update to all other listeners
	 */
	void upd_json(const cJSON * update_data);

	virtual void process_json_command(const cJSON * data);

	BaseOutput * get_truthholder();

	// Must be called to link this property up to the main
	// property handler, to avoid initialization reorders issues.
	// May also do other things, I suppose.
	virtual void init();
};

}
}