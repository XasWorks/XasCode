
#define __XNM_BLE_EXCLUDE_DIY_TYPES

#include "include/xnm/ble.h"
#include "host/ble_hs.h"

namespace XNM {
namespace BLE {

Service::Service(Server & server) : 
	local_service_id(-1), chr_pointer(nullptr),
	uuid(), server(server) {

}

ble_gatt_svc_def * Service::service_def() {
	assert(local_service_id != -1);
	
	return &(server.services[local_service_id]);
}

void Service::init() {
	server.append_service(*this);

	*(service_def()) = ble_gatt_svc_def({
		BLE_GATT_SVC_TYPE_PRIMARY,
		reinterpret_cast<ble_uuid_t*>(&uuid),
		nullptr,	// Included services, can almost always stay nullptr
		chr_pointer,	// Included characteristics, must be filled out by service specialisation
	});
}

}
}