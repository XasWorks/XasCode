
#include <xasin/mqtt.h>

// Defined here to avoid having to include esp_log.h
typedef int (*vprintf_like_t)(const char *, va_list);

#include <string>

namespace XNM {
namespace NetHelpers {

namespace OTA {
    enum ota_state_t {
        UNKNOWN,
        UP_TO_DATE,
        DOWNLOADING,
        REBOOT_NEEDED,
        UNVERIFIED,
    };

    void init();
    void cancel_rollback();

    ota_state_t get_state();

    uint32_t get_version();
    void set_upstream_version(uint32_t up_version);
}

void set_device_id();
void set_device_id(const char *name);

std::string get_device_id();

void set_mqtt(Xasin::MQTT::Handler &mqtt);
vprintf_like_t init_mqtt_logs();

void report_boot_reason();

void init_global_r3_ca();

void init();

}
}