
#include <xnm/net_helpers.h>
#include <xnm/net_helpers/r3_certificate.h>

#include <xnm/net_helpers/wifi.h>

#include <nvs.h>
#include <esp_tls.h>

#include <esp_https_ota.h>
#include <esp_ota_ops.h>

#include <lwip/apps/sntp.h>

#include <cstring>

#include <esp_log.h>

namespace XNM {
namespace NetHelpers {

Xasin::MQTT::Handler *mqtt_ptr;
std::string device_id = "";

void set_mqtt(Xasin::MQTT::Handler &mqtt) {
    mqtt_ptr = &mqtt;
}

void set_device_id() {
    uint8_t smacc[6] = {};

	char macStr[20] = {};

	esp_read_mac(smacc, ESP_MAC_WIFI_STA);

	sprintf(macStr, "%02X.%02X.%02X.%02X.%02X.%02X",
		smacc[0], smacc[1], smacc[2],
		smacc[3], smacc[4], smacc[5]);

	device_id = macStr;
}
void set_device_id(const char *str) {
    device_id = str;
}

std::string get_device_id() {
    if(device_id == "")
        set_device_id();
        
    return device_id;
}

vprintf_like_t previous_printf = nullptr;
int vprintf_like_mqtt(const char *format, va_list args) {
    auto prev_return = previous_printf(format, args);
    if(mqtt_ptr == nullptr)
        return prev_return;

    if(mqtt_ptr->is_disconnected())
        return prev_return;
    
    char printf_buffer[256] = {};
    vsnprintf(printf_buffer, sizeof(printf_buffer), format, args);

    mqtt_ptr->publish_to("logs", printf_buffer, strlen(printf_buffer));
    return strlen(printf_buffer);
}

vprintf_like_t init_mqtt_logs() {
    previous_printf = esp_log_set_vprintf(vprintf_like_mqtt);
    return previous_printf;
}

void init_global_r3_ca() {
    esp_tls_set_global_ca_store(reinterpret_cast<const unsigned char*>(lets_encrypt_rX_pem_start), lets_encrypt_rX_pem_end - lets_encrypt_rX_pem_start);
}

void report_boot_reason() {
    switch(esp_reset_reason()) {
        default: ESP_LOGW("XNM", "ESP restarted, unknown reason!"); break;
        case ESP_RST_DEEPSLEEP: ESP_LOGI("XNM", "ESP restarted after deep sleep"); break;
        case ESP_RST_POWERON: ESP_LOGI("XNM", "ESP restarted after poweron."); break;
        
        case ESP_RST_SW: ESP_LOGI("XNM", "ESP restarted due to software reset."); break;

        case ESP_RST_PANIC: ESP_LOGE("XNM", "ESP restarted due to kernel panic!"); break;
        
        case ESP_RST_INT_WDT:
        case ESP_RST_TASK_WDT:
        case ESP_RST_WDT:
            ESP_LOGE("XNM", "ESP restarted due to a watchdog panic!");
        break;

        case ESP_RST_BROWNOUT: ESP_LOGE("XNM", "ESP restarted due to a brownout!"); break;

        case ESP_RST_EXT:
        case ESP_RST_SDIO:
            ESP_LOGI("XNM", "ESP restarted, external.");
        break;
    }
}

void init_sntp() {
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_set_sync_mode(SNTP_SYNC_MODE_IMMED);
    sntp_setservername(0, "pool.ntp.org\0");

    sntp_init();
}

void init() {
#ifdef CONFIG_XNM_AUTOSTART_WIFI
    WIFI::init();
    init_sntp();
#endif

#ifdef CONFIG_AUTOSTART_MQTT_LOG_REDIR
    init_mqtt_logs();
    esp_log_level_set("TRANS_TCP", ESP_LOG_NONE);
#endif

#ifdef CONFIG_AUTOSTART_OTA
    OTA::init();
#endif
}

}
}