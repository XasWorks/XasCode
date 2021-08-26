
#pragma once

namespace XNM {
namespace NetHelpers {
namespace WIFI {

enum wifi_state_t {
	UNINITIALIZED,				// init() has not yet been called
	DISABLED,					// the WIFI system has been disabled completely
	DISCONNECTED,				// Disconnected and waiting to restart search
	RETRYING,
	CONNECTING,					// Waiting to connect to an AP
	CONNECTED,					// Connected and happy :>
	SOFT_AP						// Soft AP was started instead to self-host
};

void init();

// Call this function approximately once a second to allow the
// WiFi system to properly handle disconnects, timeouts, etc.
void housekeep_tick();

// If NVS storage is enabled, this will save the given string
// to NVS.
void set_nvs(const char *ssid, const char * password);

}
}
}