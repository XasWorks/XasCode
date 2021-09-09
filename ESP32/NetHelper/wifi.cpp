

#include "include/xnm/net_helpers/wifi.h"

#include <cstring>
#include <algorithm>

#include "freertos/FreeRTOS.h"

#include "esp_event.h"
#include "esp_system.h"
#include "esp_wifi.h"

#include "nvs_flash.h"

#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG
#include "esp_log.h"

namespace XNM {
namespace NetHelpers {
namespace WIFI {

const char * wifi_tag = "XNM::WiFi";

wifi_state_t state = UNINITIALIZED;

TickType_t timeout_tick = 0;
uint32_t   retry_count = 0;
bool managed_connection = false;

char sta_ssid[64] = {};
char sta_password[64] = {};

int get_max_retry_count() {
#ifdef CONFIG_XNM_WIFI_CONNECTED_MAX_RETRIES
	if(managed_connection)
		return CONFIG_XNM_WIFI_CONNECTED_MAX_RETRIES;
#endif

	return CONFIG_XNM_WIFI_FIRST_MAX_RETRIES;
}

void config_sta() {
	ESP_LOGI(wifi_tag, "Configuring station mode");

	esp_wifi_stop();
	esp_wifi_set_mode(WIFI_MODE_STA);

	managed_connection = false;
	retry_count = 0;

	if(CONFIG_XNM_WIFI_PSMODE >= 2)
			esp_wifi_set_max_tx_power(50);
	if(CONFIG_XNM_WIFI_PSMODE >= 1)
		ESP_ERROR_CHECK( esp_wifi_set_ps(WIFI_PS_MAX_MODEM));
	if(CONFIG_XNM_WIFI_PSMODE == -1)
		ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_NONE));

	wifi_config_t wifi_cfg = {};
	wifi_sta_config_t* sta_cfg = &(wifi_cfg.sta);

	memcpy(sta_cfg->password, sta_password, strlen(sta_password));
	memcpy(sta_cfg->ssid, sta_ssid, strlen(sta_ssid));

	if(CONFIG_XNM_WIFI_PSMODE >= 2)
		sta_cfg->listen_interval = 10;

	esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_cfg);
}

/*void config_ap() {
	ESP_LOGI(wifi_tag, "Configuring ap mode");

	esp_wifi_stop();
	esp_wifi_set_mode(WIFI_MODE_AP);

	wifi_config_t wifi_cfg = {};
	wifi_ap_config_t & ap_cfg = (wifi_cfg.ap);

	if(strlen(CONFIG_XNM_WIFI_AP_SSID) > 0)
		strcpy(reinterpret_cast<char *>(ap_cfg.ssid), CONFIG_XNM_WIFI_AP_SSID);
	else
		snprintf(reinterpret_cast<char *>(ap_cfg.ssid), 32, "%s-AP", CONFIG_PROJECT_NAME);

	if(strlen(CONFIG_XNM_WIFI_AP_PASSWORD)) {

		strcpy(reinterpret_cast<char *>(ap_cfg.password), CONFIG_XNM_WIFI_AP_PASSWORD);
		ap_cfg.authmode = WIFI_AUTH_WPA2_PSK;
	}
	else
		ap_cfg.authmode = WIFI_AUTH_OPEN;

	ap_cfg.max_connection = 4;
	ap_cfg.beacon_interval = 500;

	esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_cfg);

}

void open_ap() {
	wifi_mode_t mode;
	esp_wifi_get_mode(&mode);

	if(mode != WIFI_MODE_AP)
		config_ap();

	state = SOFT_AP;

	esp_wifi_start();
}*/

void try_connect_sta() {
	wifi_mode_t mode;
	esp_wifi_get_mode(&mode);

	if(mode != WIFI_MODE_STA)
		config_sta();

	ESP_LOGI(wifi_tag, "Trying to connect!");

	state = CONNECTING;
	timeout_tick = xTaskGetTickCount() + 5000/portTICK_PERIOD_MS;
	
	retry_count = 1;

	esp_wifi_start();
}
void try_reconnect_sta() {
	retry_count++;
	state = CONNECTING;
	timeout_tick = xTaskGetTickCount() + 5000/portTICK_PERIOD_MS;

	ESP_LOGW(wifi_tag, "Trying to reconnect (%d/%d)", retry_count, get_max_retry_count());

	esp_wifi_connect();
}
void on_sta_connected() {
	ESP_LOGI(wifi_tag, "Network connected!");

	retry_count = 0;
	state = CONNECTED;
	managed_connection = true;
}


void on_sta_con_failed() {
	if(state == CONNECTED)
		ESP_LOGW(wifi_tag, "Disconnected!");
	else
		ESP_LOGD(wifi_tag, "Disconnected");

	auto max_retries = get_max_retry_count();
	if((max_retries > 0) && (retry_count >= max_retries)) {
		
		ESP_LOGW(wifi_tag, "Maximum number of WiFi connection retries reached!");
		state = DISCONNECTED;

		return;
	}

	state = RETRYING;

	TickType_t retry_time = std::min<uint32_t>(CONFIG_XNM_WIFI_MAX_RETRY_TIME, 
		std::max<uint32_t>(CONFIG_XNM_WIFI_MIN_RETRY_TIME, (100*retry_count*retry_count)));

	ESP_LOGD(wifi_tag, "Reconnecting in %dms", retry_time);

	timeout_tick = xTaskGetTickCount() + retry_time/portTICK_PERIOD_MS;
}

void housekeep_tick() {
	switch(state) {
	default: return;

	case RETRYING:
		if(xTaskGetTickCount() >= timeout_tick)
			try_reconnect_sta();
	break;

	case CONNECTING:
		if(xTaskGetTickCount() >= timeout_tick)
			on_sta_con_failed();
	}
}

void event_handler(void * arg, esp_event_base_t event_base,
	int32_t event_id, void * event_data) {
	
	if(event_base == WIFI_EVENT) {
		switch(event_id) {
		case WIFI_EVENT_STA_START:
				ESP_LOGD(wifi_tag, "STA START");
				esp_wifi_connect();
		break;

		case WIFI_EVENT_STA_DISCONNECTED:
			ESP_LOGD(wifi_tag, "STA DISCONNECT");
			on_sta_con_failed();
		break;
		}
	}

	else if(event_base == IP_EVENT) {
		if(event_id == IP_EVENT_STA_GOT_IP)
			on_sta_connected();
	}
}

void set_nvs(const char *ssid, const char *password) {
	nvs_handle_t write_handle;
	nvs_open("xnm", NVS_READWRITE, &write_handle);

	nvs_set_str(write_handle, "w_ssid", ssid);
	nvs_set_str(write_handle, "w_pswd", password);

	nvs_commit(write_handle);

	nvs_close(write_handle);
}

void load_nvs() {
	static bool nvs_data_loaded = false;
	if(nvs_data_loaded)
		return;

	nvs_data_loaded = true;

	size_t write_len = 64;

	nvs_handle_t read_handle;

	nvs_open("xnm", NVS_READONLY, &read_handle);
	auto ret = nvs_get_str(read_handle, "w_ssid", sta_ssid, &write_len);

	if(ret != ESP_OK) {
		nvs_close(read_handle);

		sta_ssid[0] = 0;
		return;
	}

	write_len = 64;
	ret = nvs_get_str(read_handle, "w_pswd", sta_password, &write_len);
	
	nvs_close(read_handle);

	if(ret != ESP_OK) {
		sta_ssid[0] = 0;
		sta_password[0] = 0;
	}
}

bool init(bool blocking) {
	esp_netif_init();

   esp_netif_create_default_wifi_sta();

	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();

	ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
	ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_FLASH) );
	

#if CONFIG_XNM_WIFI_LOAD_NVS
	load_nvs();
#endif

	if(strlen(CONFIG_XNM_WIFI_DEFAULT_SSID) 
		&& sta_ssid[0] == 0)
		strcpy(sta_ssid, CONFIG_XNM_WIFI_DEFAULT_SSID); 

	if(strlen(CONFIG_XNM_WIFI_DEFAULT_PASSWORD) > 0 
		&& sta_password[0] == 0)
		strcpy(sta_password, CONFIG_XNM_WIFI_DEFAULT_PASSWORD);

	ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT,
															ESP_EVENT_ANY_ID,
															&event_handler,
															NULL));
	ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT,
															IP_EVENT_STA_GOT_IP,
															&event_handler,
															NULL));


	if(sta_ssid[0] != 0)
		try_connect_sta();
	else
		return false;

	if(!blocking)
		return true;

	while(true) {
		if(state == CONNECTED)
			return true;
		if(state == DISCONNECTED)
			return false;

		vTaskDelay(100/portTICK_PERIOD_MS);
	}
}

bool has_config() {
#if CONFIG_XNM_WIFI_LOAD_NVS
	load_nvs();
#endif

	return (sta_ssid[0] != 0) && (sta_password[0] != 0);
}

bool is_connected() {
	return state == CONNECTED;
}

bool should_autostart() {
#ifdef CONFIG_XNM_WIFI_AUTOSTART
	return true;
#endif

	// TODO add link to OTA to check if it wants a download

	return false;
}

}
}
}
