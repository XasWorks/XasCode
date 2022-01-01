
#include "include/xnm/property_point/UARTOutput.h"

#include <esp_log.h>

#include <cstring>

namespace XNM {
namespace PropertyPoint {

void UARTOutput::_call_line_task(void *args) {
	assert(args);

	reinterpret_cast<UARTOutput*>(args)->line_task();
}

UARTOutput::UARTOutput(Handler &handler) : BaseOutput(handler),
	line_task_handle(nullptr) {
}

void UARTOutput::send_upd_json(const cJSON * item, BaseProperty &prop) {
	cJSON * out_obj = cJSON_CreateObject();

	cJSON * upd_obj = cJSON_CreateObject();
	cJSON_AddItemToObjectCS(out_obj, "upd", upd_obj);

	cJSON_AddItemReferenceToObject(upd_obj, prop.key, const_cast<cJSON*>(item));

	char * out = cJSON_PrintUnformatted(out_obj);


	printf("P %s\n", out);

	cJSON_free(out);
	cJSON_Delete(out_obj);
}

void UARTOutput::init() {
	assert(line_task_handle == nullptr);

	xTaskCreate(_call_line_task, "PP:UART_RX", 3*1024, this, 1, &line_task_handle);


}

void UARTOutput::line_task() {
	char * in_buffer = new char[1024];

	memset(in_buffer, 0, 1024);

	while(true) {
		in_buffer[0] = 0;
		char *in_ptr = in_buffer;

		while(in_ptr < (in_buffer + 1023)) {
			auto c = getchar();

			if(c == EOF) {
				vTaskDelay(100/portTICK_PERIOD_MS);
				continue;
			}

			if(c == 0 || c == '\n') {
				*in_ptr = 0;
				break;
			}

			*in_ptr = c;
			in_ptr++;
		}

		ESP_LOGI("TEST", "Got line: %s", in_buffer);

		handler.process_command_json(in_buffer, *this);
	}
}

}
}