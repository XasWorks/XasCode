
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

// #define LOG_LOCAL_LEVEL ESP_LOG_DEBUG
#include <esp_log.h>

namespace XNM {
namespace NetHelpers {

extern Xasin::MQTT::Handler * mqtt_ptr;

namespace OTA {
	ota_version_t::ota_version_t() { u32 = 0; }
	ota_version_t::ota_version_t(uint32_t int_version) {
		u32 = int_version;
	}

	ota_state_t ota_state = UNKNOWN;

	TaskHandle_t current_update_thread = nullptr;

	ota_version_t current_version = {};
	ota_version_t upstream_version = {};

	const char * branch_name = "main";

	const char * get_branch_name() {
		return branch_name;
	}

	bool compare_version(ota_version_t higher_version, ota_version_t current_version) {
		if(higher_version.u32 == 0)
			return false;

		if(higher_version.release_ver && current_version.release_ver) {
			if(higher_version.year_vers > current_version.year_vers)
				return true;
			if(higher_version.year_vers < current_version.year_vers)
				return false;

			if(higher_version.month_vers > current_version.month_vers)
				return true;
			if(higher_version.month_vers < current_version.month_vers)
				return false;

			return higher_version.minor_vers > current_version.minor_vers;
		}

		return higher_version.u32 != current_version.u32;
	}

	void print_version(char * buffer, size_t b_size, ota_version_t vers) {
		snprintf(buffer, b_size, "%s%4d.%02d.%04d", vers.release_ver ? "v" : "vD",
			vers.year_vers + 2000, vers.month_vers, vers.minor_vers);
	}

	ota_version_t get_local_version() {
		if(current_version.u32 != 0)
			return current_version;

		nvs_handle_t nvs;
		nvs_open("xnmota", NVS_READONLY, &nvs);

		uint32_t out = 0;
		auto ret = nvs_get_u32(nvs, "local_v", &out);

		nvs_close(nvs);

		if(ret != ESP_OK)
			return 0;

		current_version.u32 = out;
		
		return out;
	}

	ota_version_t get_nvs_upstream_version() {
		if(upstream_version.u32 != 0)
			return upstream_version;

		nvs_handle_t nvs;
		nvs_open("xnmota", NVS_READONLY, &nvs);

		uint32_t out = 0;
		auto ret = nvs_get_u32(nvs, "upstream_v", &out);

		nvs_close(nvs);

		if(ret != ESP_OK)
			return 0;

		upstream_version.u32 = out;
		
		return out;
	}

	ota_version_t get_remove_version() {
		return get_nvs_upstream_version();
	}

	ota_version_t pull_upstream_version() {
		ota_version_t out_version = 0;

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


				auto o_vers = std::strtol(bfr, nullptr, 10);
				out_version.u32 = o_vers;

				print_version(bfr, 255, out_version);

				ESP_LOGD("OTA", "Got a response of %d bytes (%ld), new vers. is %s", data_read, o_vers, bfr);
			}
		}

		esp_http_client_close(client);
		esp_http_client_cleanup(client);

		return out_version;
	}

	void set_local_version(ota_version_t vers) {
		current_version = vers;

		nvs_handle_t nvs;
		nvs_open("xnmota", NVS_READWRITE, &nvs);

		nvs_set_u32(nvs, "local_v", vers.u32);
		nvs_close(nvs);
	}

	void report_version() {
		char cvers_buffer[64];
		char upstr_buffer[64];

		print_version(cvers_buffer, sizeof(cvers_buffer), get_local_version());
		print_version(upstr_buffer, sizeof(upstr_buffer), get_nvs_upstream_version());

		if(compare_version(get_nvs_upstream_version(), get_local_version())) {
			ESP_LOGW("OTA", "Local version %s - update available (%s)!", cvers_buffer, upstr_buffer);
		}
		else {
			ESP_LOGI("OTA", "Local version %s - no update available (%s).", cvers_buffer, upstr_buffer);
		}
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

	void set_upstream_version(ota_version_t up_version) {
		if(up_version.u32 == 0)
			return;

		esp_ota_mark_app_valid_cancel_rollback();
		ota_state = UP_TO_DATE;

		if(!compare_version(up_version, upstream_version))
			return;		

		upstream_version = up_version;

		if(compare_version(upstream_version, get_local_version()))
			ota_state = UPDATE_AVAILABLE;
	}

	void housekeep_tick() {
		static TickType_t ota_pull_delay = xTaskGetTickCount() + esp_random() % ((1000*3*60)/portTICK_PERIOD_MS);

		if(get_nvs_upstream_version().release_ver && get_local_version().release_ver && (ota_pull_delay > xTaskGetTickCount()))
			return;

		ota_pull_delay += (1000*3*60)/portTICK_PERIOD_MS;

		if(OTA::get_state() == OTA::UPDATE_AVAILABLE) {
			ESP_LOGW("OTA", "Going to restart to run the update!");

			vTaskDelay(100);
			
			esp_restart();
      }
	}

	void init() {
		static bool ota_initialized = false;
		if(ota_initialized) {
			ESP_LOGE("OTA", "OTA Set up twice, ignoring second one!");
			return;
		}

		if(esp_get_free_heap_size() < 32000) {
			ESP_LOGE("OTA", "Minimum free heap for safe OTA was not reached, rolling back!");

			esp_ota_mark_app_invalid_rollback_and_reboot();
		}

#ifdef CONFIG_XNM_OTA_BOOTCHECK
		auto pulled_vers = pull_upstream_version();
		if(pulled_vers.u32 != 0)
			set_upstream_version(pulled_vers);
#endif

		char cvers_buffer[64];
		char upstr_buffer[64];

		print_version(cvers_buffer, sizeof(cvers_buffer), get_local_version());
		print_version(upstr_buffer, sizeof(upstr_buffer), get_nvs_upstream_version());

		if(compare_version(get_nvs_upstream_version(), get_local_version())) {
			ESP_LOGW("OTA", "Local version %s - update available (%s), performing update!", cvers_buffer, upstr_buffer);
			
			perform_https_ota();
		}

		ota_initialized = true;
	}

	ota_state_t get_state() {
		return ota_state;
	}

	void force_rollback() {
		esp_ota_mark_app_invalid_rollback_and_reboot();
	}

}

}
}