
#pragma once


#include "host/ble_gatt.h"
#include "host/ble_gap.h"

#include <vector>

namespace XNM {
namespace BLE {

class Service;

class Server {
private:
	static int static_gap_cb(ble_gap_event *event, void *arg);
	int gap_cb(ble_gap_event *event);

protected:
friend Service;

	int mtu;

	std::vector<ble_gatt_svc_def> services;
	void append_service(Service & service);

	bool is_connected;
	bool should_advertise;

public:
	Server();

	void init();

	void start_gatt();
	
	void start_advertising(const char * name = "TEST NAME");
};

}
}