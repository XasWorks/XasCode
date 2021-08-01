
#include "xnm/property_point/BaseOutput.h"
#include "xnm/property_point/BaseHandler.h"

namespace XNM {
namespace PropertyPoint {

BaseOutput::BaseOutput(Handler & handler) : handler(handler) {
	handler.insert_output(*this);
}

void BaseOutput::send_upd_json(const cJSON * item, BaseProperty &prop) {}
void BaseOutput::send_set_json(const cJSON * item, BaseProperty &prop) {}

}
}