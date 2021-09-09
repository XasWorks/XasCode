
/*
 *	TODO: Add NVS or RTC-memory based OTA triggers
 *       Furthermore, make the main pull URL configurable
 *
 */

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

	int32_t current_version = 0;
	int32_t upstream_version = 0;

	const char * branch_name = "main";

	const char * get_branch_name() {
		return branch_name;
	}

	int32_t get_local_version() {
		if(current_version != 0)
			return current_version;

		nvs_handle_t nvs;
		nvs_open("xnmota", NVS_READONLY, &nvs);

		int32_t out = 0;
		auto ret = nvs_get_i32(nvs, "local_v", &out);

		nvs_close(nvs);

		if(ret != ESP_OK)
			return 0;

		current_version = out;
		
		return out;
	}

	int32_t get_nvs_upstream_version() {
		if(upstream_version != 0)
			return upstream_version;

		nvs_handle_t nvs;
		nvs_open("xnmota", NVS_READONLY, &nvs);

		int32_t out = 0;
		auto ret = nvs_get_i32(nvs, "upstream_v", &out);

		nvs_close(nvs);

		if(ret != ESP_OK)
			return 0;

		upstream_version = out;
		
		return out;
	}

	int32_t pull_upstream_version() {
		uint32_t out_version = -1;

		char bfr[255] = {};
		snprintf(bfr, sizeof(bfr) - 1, "https://xaseiresh.hopto.org/api/esp_ota/%s/%s.vers", CONFIG_PROJECT_NAME, branch_name);

		esp_http_client_config_t version_cfg = {};
		version_cfg.url = bfr;
		version_cfg.use_global_ca_store = true;
		version_cfg.cert_pem = lets_encrypt_rX_pem_start;
	
		esp_http_client_handle_t client = esp_http_client_init(&version_cfg);

		// GET Request
		esp_http_client_set_method(client, HTTP_METHOD_GET);
		esp_err_t err = esp_http_client_open(client, 0);
		
		ESP_LOGD("OTA", "Upstream fetch open returned: %d", err);

		if (err == ESP_OK) {
			size_t content_length = esp_http_client_fetch_headers(client);
			ESP_LOGD("OTA", "Content length is %d bytes!", content_length);

			if (content_length > 0) {
				bfr[0] = 0;

				int data_read = esp_http_client_read(client, bfr, sizeof(bfr));
				
				bfr[data_read] = 0;

				ESP_LOGD("OTA", "Got a response of %d bytes (%s)", data_read, bfr);

				out_version = std::strtol(bfr, nullptr, 10);
			}
		}

		esp_http_client_close(client);
		esp_http_client_cleanup(client);

		return out_version;
	}

	void set_local_version(int32_t vers) {
		current_version = vers;

		nvs_handle_t nvs;
		nvs_open("xnmota", NVS_READWRITE, &nvs);

		nvs_set_i32(nvs, "local_v", vers);
		nvs_close(nvs);
	}

	void perform_https_ota() {
		ota_state = DOWNLOADING;

		char bfr[255] = {};
		snprintf(bfr, 128, "https://xaseiresh.hopto.org/api/esp_ota/%s/%s.bin", CONFIG_PROJECT_NAME, branch_name);

		esp_http_client_config_t ota_cfg = {};
		ota_cfg.url = bfr;
		ota_cfg.use_global_ca_store = true;
		ota_cfg.cert_pem = lets_encrypt_rX_pem_start;

		auto ret = esp_https_ota(&ota_cfg);

		set_local_version(upstream_version);

		vTaskDelay(100/portTICK_PERIOD_MS);

		esp_restart();
	}

	void set_upstream_version(int32_t up_version) {
		if(up_version == 0)
			return;
		if(up_version <= upstream_version)
			return;
		
		esp_ota_mark_app_valid_cancel_rollback();

		upstream_version = up_version;

		if(upstream_version > current_version) {
			ota_state = UPDATE_AVAILABLE;
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

#ifdef CONFIG_XNM_OTA_BOOTCHECK
		auto pulled_vers = pull_upstream_version();
		if(pulled_vers > 0)
			set_upstream_version(pulled_vers);
#endif

		if(get_nvs_upstream_version() > get_local_version()) {
			ESP_LOGW("OTA", "Upstream version %d, local %d - performing update!", get_nvs_upstream_version(), get_local_version());
			perform_https_ota();
		}
		else
			ESP_LOGI("OTA", "Upstream version %d, local %d - no update needed.", get_nvs_upstream_version(), get_local_version());

		ota_initialized = true;
	}

	ota_state_t get_state() {
		return ota_state;
	}
}

}
}