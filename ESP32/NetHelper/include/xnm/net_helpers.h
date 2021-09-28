#pragma once

#include <xasin/mqtt.h>

// Defined here to avoid having to include esp_log.h
typedef int (*vprintf_like_t)(const char *, va_list);

#include <string>
#include "net_helpers/wifi.h"
#include "net_helpers/ota.h"

#include <xnm/ble.h>

#include <xnm/property_point.h>

namespace XNM {
namespace NetHelpers {

enum net_state_t {
    UNINITIALIZED,
    BLE_MODE,           // Used when no wifi config is present and config mode was entered
    WIFI_CONNECTING,    
    WIFI_CONNECTED,     // Used when WiFi connection was confirmed       
    DOWNLOADING_OTA,
    NETWORK_MODE,       // Used when WiFi and MQTT have been confirmed and normal operation proceeds
};

#ifdef CONFIG_XNM_NETHELP_MQTT_ENABLE
extern Xasin::MQTT::Handler mqtt;
#endif

#ifdef CONFIG_XNM_NETHELP_BLE_ENABLE
extern XNM::BLE::Server ble;
#endif

#ifdef CONFIG_XNM_NETHELP_INCLUDE_PROPP
extern XNM::PropertyPoint::Handler propp;
#endif

net_state_t get_state();

void event_handler(system_event_t *event);

void set_device_id();
void set_device_id(const char *name);

std::string get_device_id();

vprintf_like_t init_mqtt_logs();
// std::function<void (const char*)> also_print_to;

void report_boot_reason();

void init_global_r3_ca();

void init();

}
}