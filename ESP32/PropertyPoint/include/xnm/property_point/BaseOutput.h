
#pragma once

#include <cJSON.h>

namespace XNM {
namespace PropertyPoint {

class Handler;
class BaseProperty;

class BaseOutput {
protected:
friend Handler;
friend BaseProperty;

	Handler &handler;

	/* Send a update command to this output
	 *
	 *	This will forward the given cJSON update structure to this output,
	 * which is intended to tell listeners on this output about the changed
	 * state of this item.
	 * 
	 * This is intended to be sent to all outputs except the truthholder output,
	 * as it will already know about the updated value!
	 * 
	 * Will *not* delete the cJSON item when done.
	 */
	virtual void send_upd_json(const cJSON * item, BaseProperty &prop);

	/*	Send a set command to this output.
	 *
	 * This will forward the given cJSON Item for the property 'key' to this
	 * output.
	 * This is supposed to be used only to forward the packet towards the
	 * truthholder, i.e. the source that controls the current value.
	 * 
	 * Will *not* delete the cJSON item when done.
	 */
	virtual void send_set_json(const cJSON * item, BaseProperty &prop);

public:
	BaseOutput(Handler &handler);
};

}
}