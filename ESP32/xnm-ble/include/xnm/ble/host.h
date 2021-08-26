
#pragma once

#include <vector>

#include "host/ble_hs.h"

namespace XNM {
namespace BLE {

static const char * tag = "xnmble";

class Service;

class Server {
protected:
friend Service;

	std::vector<ble_gatt_svc_def> services;

	void append_service(Service & service);
public:
	Server();

	void init();

	void start_gatt();
	
	void start_advertising(const char * name = "TEST NAME");

	void DBG_send_str(const char *str);
};

}
}