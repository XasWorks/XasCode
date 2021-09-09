
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

net_state_t net_state = UNINITIALIZED;
std::string device_id = "";

#ifdef CONFIG_XNM_NETHELP_MQTT_ENABLE
Xasin::MQTT::Handler mqtt;
#endif

#ifdef CONFIG_XNM_NETHELP_BLE_ENABLE
XNM::BLE::Server ble;
#endif

void event_handler(system_event_t *event) {
#ifdef XNM_NETHELP_MQTT_ENABLE
    mqtt.wifi_handler(event);
#endif
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

#ifdef CONFIG_XNM_NETHELP_MQTT_ENABLE
vprintf_like_t previous_printf = nullptr;
int vprintf_like_mqtt(const char *format, va_list args) {
    static volatile uint8_t print_nest_count = 0;

    auto prev_return = previous_printf(format, args);

    if(mqtt.is_disconnected())
        return prev_return;
    
    print_nest_count++;
    
    char printf_buffer[256] = {};
    if(print_nest_count < 2) {
        vsnprintf(printf_buffer, sizeof(printf_buffer), format, args);

        mqtt.publish_to("logs", printf_buffer, strlen(printf_buffer));
    }
    
    print_nest_count--;
    return strlen(printf_buffer);
}

vprintf_like_t init_mqtt_logs() {
    previous_printf = esp_log_set_vprintf(vprintf_like_mqtt);
    return previous_printf;
}

#else // If we do not have MQTT running, do nothing to initialize MQTT logging
void init_mqtt_logs() {}
#endif

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

#ifdef CONFIG_XNM_NETHELP_BLE_ENABLE
void init_ble() {
    static bool ble_was_started = false;
    if(ble_was_started)
        return;

    net_state = BLE_MODE;

    ble.init();
    ble.start_advertising();

    ble_was_started = true;

#ifdef CONFIG_XNM_NETHELP_BLE_CONFMODE_BLOCK
    while(true) {
        vTaskDelay(portMAX_DELAY);
    }
#endif
}
#else
void init_ble() {}
#endif

#ifdef CONFIG_XNM_NETHELP_MQTT_ENABLE
void init_mqtt() {
    mqtt.start_from_nvs();

    TickType_t start_tick = xTaskGetTickCount();

    while(true) {
        vTaskDelay(50/portTICK_PERIOD_MS);

        if(mqtt.is_disconnected() == 0) {
            net_state = NETWORK_MODE;
            break;
        }

        if((xTaskGetTickCount() - start_tick) > (CONFIG_XNM_NETHELP_MQTT_CON_TIME / portTICK_PERIOD_MS)) {
            mqtt.stop();
            return;
        }
    }

#ifdef CONFIG_AUTOSTART_MQTT_LOG_REDIR
    esp_log_level_set("TRANS_TCP", ESP_LOG_NONE);
    init_mqtt_logs();
#endif
}
#else
void init_mqtt() {
    net_state = NETWORK_MODE;
}
#endif

void nethelp_housekeep_tick(void *arg) {
    while(true) {
        vTaskDelay(1000/portTICK_PERIOD_MS);

        WIFI::housekeep_tick();
    }
}

void init() {
    xTaskCreate(nethelp_housekeep_tick, "XNM::Housekeep",
        2048, nullptr, 3, nullptr);

#ifdef CONFIG_XNM_NETHELP_BLE_ALWAYSON
    init_ble();
#endif

#ifdef CONFIG_XNM_NETHELP_BLE_AUTOCONF
    if(!WIFI::has_config()) {
        init_ble();
        return;
    }
#endif

    if(WIFI::should_autostart()) {
        net_state = WIFI_CONNECTING;

        if(WIFI::init(true)) {
            net_state = WIFI_CONNECTED;
            init_sntp();
        }
    }

#ifdef CONFIG_XNM_NETHELP_BLE_NONETWORK
    if(net_state != WIFI_CONNECTED) {
        init_ble();
        return;
    }
#endif

    OTA::init();

    init_mqtt();

#ifdef CONFIG_XNM_NETHELP_BLE_NONETWORK
    if(net_state != NETWORK_MODE) {
        init_ble();
        return;
    }
#endif
}

}
}