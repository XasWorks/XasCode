

#pragma once

#include "host/ble_gatt.h"

namespace XNM {
namespace BLE {

class Server;

class Service {
protected:
friend Server;

	// Local ID of this service, used to fetch
	// the service struct from the host.
	// We can't use a pointer or reference as the std::vector might
	// reallocate!
	int local_service_id;

	ble_gatt_chr_def * chr_pointer;

	ble_gatt_svc_def * service_def();

	// Pointer to the UUID of this service.
	ble_uuid_any_t uuid;

public:
	Server & server;
	Service(Server & server);

	virtual void init();
};

}
}