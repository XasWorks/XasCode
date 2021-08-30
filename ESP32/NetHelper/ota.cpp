

#include <xnm/net_helpers.h>
#include <xnm/net_helpers/r3_certificate.h>

#include <nvs.h>

#include <esp_http_client.h>
#include <esp_https_ota.h>
#include <esp_ota_ops.h>

#include <cJSON.h>

#include <esp_log.h>

namespace XNM {
namespace NetHelpers {

extern Xasin::MQTT::Handler * mqtt_ptr;

namespace OTA {
	ota_state_t ota_state = UNKNOWN;

	TaskHandle_t current_update_thread = nullptr;

	uint32_t current_version = 0;
	uint32_t upstream_version = 0;

	const char * branch_name = "main";

	const char * get_branch_name() {
		return branch_name;
	}

	uint32_t get_local_version() {
		if(current_version != 0)
			return current_version;

		nvs_handle_t nvs;
		nvs_open("xnmota", NVS_READONLY, &nvs);

		uint32_t out = 0;
		auto ret = nvs_get_u32(nvs, "local_v", &out);

		nvs_close(nvs);

		if(ret != ESP_OK)
			return 0;

		current_version = out;
		
		return out;
	}

	uint32_t get_nvs_upstream_version() {
		if(upstream_version != 0)
			return upstream_version;

		nvs_handle_t nvs;
		nvs_open("xnmota", NVS_READONLY, &nvs);

		uint32_t out = 0;
		auto ret = nvs_get_u32(nvs, "upstream_v", &out);

		nvs_close(nvs);

		if(ret != ESP_OK)
			return 0;

		upstream_version = out;
		
		return out;
	}

	uint32_t pull_upstream_version() {
		uint32_t out_version = -1;

		char bfr[255] = {};
		snprintf(bfr, 128, "https://xaseiresh.hopto.org/api/esp_ota/%s.vers", CONFIG_PROJECT_NAME);

		esp_http_client_config_t version_cfg = {};
		version_cfg.url = bfr;
		version_cfg.use_global_ca_store = true;
		version_cfg.cert_pem = lets_encrypt_rX_pem_start;
	
		esp_http_client_handle_t client = esp_http_client_init(&version_cfg);

		// GET Request
		esp_http_client_set_method(client, HTTP_METHOD_GET);
		esp_err_t err = esp_http_client_open(client, 0);
		
		if (err == ESP_OK) {
			size_t content_length = esp_http_client_fetch_headers(client);
			
			if (content_length > 0) {
				bfr[0] = 0;

				int data_read = esp_http_client_read(client, bfr, sizeof(bfr));
				
				cJSON * version_data = cJSON_Parse(bfr);
				
				cJSON * version_number = cJSON_GetObjectItem(version_data, get_branch_name());

				if(cJSON_IsNumber(version_number))
					out_version = version_number->valueint;

				cJSON_Delete(version_data);
			}
		}

		esp_http_client_close(client);
		esp_http_client_cleanup(client);

		return out_version;
	}

	void set_local_version(uint32_t vers) {
		current_version = vers;

		nvs_handle_t nvs;
		nvs_open("xnmota", NVS_READWRITE, &nvs);

		nvs_set_u32(nvs, "local_v", vers);
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

		set_local_version(upstream_version);

		ota_state = REBOOT_NEEDED;

		vTaskDelay(10);

		esp_restart();

		vTaskDelay(10);
		
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
		
		get_local_version();

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

}
}