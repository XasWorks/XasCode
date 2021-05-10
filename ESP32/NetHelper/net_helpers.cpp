
#include <xnm/net_helpers.h>
#include <xnm/net_helpers/r3_certificate.h>

#include <nvs.h>
#include <esp_tls.h>

#include <esp_https_ota.h>
#include <esp_ota_ops.h>

#include <cstring>

#include <esp_log.h>

namespace XNM {
namespace NetHelpers {

Xasin::MQTT::Handler *mqtt_ptr;
std::string device_id = "";

namespace OTA {
    ota_state_t ota_state = UNKNOWN;

    TaskHandle_t current_update_thread = nullptr;

    uint32_t current_version = 0;
    uint32_t upstream_version = 0;

    uint32_t get_version() {
        if(current_version != 0)
            return current_version;

        nvs_handle_t nvs;
        nvs_open("xasin", NVS_READONLY, &nvs);

        uint32_t out = 0;
        auto ret = nvs_get_u32(nvs, "ota_vers", &out);

        nvs_close(nvs);

        if(ret != ESP_OK)
            return 0;

        current_version = out;
        return out;
    }

    void set_version(uint32_t vers) {
        current_version = vers;

        nvs_handle_t nvs;
        nvs_open("xasin", NVS_READWRITE, &nvs);

        nvs_set_u32(nvs, "ota_vers", vers);
        nvs_close(nvs);
    }

    void esp_ota_thread(void *args) {
        char bfr[128] = {};
        snprintf(bfr, 128, "https://xaseiresh.hopto.org/api/esp_ota/%s.bin", CONFIG_PROJECT_NAME);

        esp_http_client_config_t ota_cfg = {};
        ota_cfg.url = bfr;
        ota_cfg.use_global_ca_store = true;
        ota_cfg.cert_pem = lets_encrypt_rX_pem_start;

        esp_https_ota(&ota_cfg);

        set_version(upstream_version);

        ota_state = REBOOT_NEEDED;

        esp_restart();

        vTaskDelete(0);
    }

    void set_upstream_version(uint32_t up_version) {
        if(up_version == 0)
            return;
        if(up_version <= upstream_version)
            return;
        
        esp_ota_mark_app_valid_cancel_rollback();

        upstream_version = up_version;

        if(upstream_version > current_version) {
            ota_state = DOWNLOADING;
            xTaskCreatePinnedToCore(esp_ota_thread, "OTA Thread", 6144, nullptr, 1, nullptr, 0);
        }
        else
            ota_state = UP_TO_DATE;
    }

    void init() {
        static bool ota_initialized = false;

        if(ota_initialized) {
            ESP_LOGE("OTA", "OTA Set up twice, ignoring second one!");
            return;
        }
        ota_initialized = true;
        
        get_version();

        char buf[128] = {};
        snprintf(buf, 128, "/esp32/%s/ota", CONFIG_PROJECT_NAME);

        if(mqtt_ptr == nullptr) {
            ESP_LOGE("XNM OTA", "No MQTT was provided, no OTA fetch possible!");
            return;
        }

        mqtt_ptr->subscribe_to(buf, [](const Xasin::MQTT::MQTT_Packet data) {
            set_upstream_version(std::stoul(data.data));
        });

        ota_initialized = true;
    }

    ota_state_t get_state() {
        return ota_state;
    }
}

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

void init() {
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