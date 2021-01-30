/*
 * RX.cpp
 *
 *  Created on: 12 Dec 2020
 *      Author: xasin
 */

#include "xasin/audio/RX.h"

#include <cmath>
#include <cstring>

#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE
#include <esp_log.h>

namespace Xasin {
namespace Audio {

void call_rx_dma_read(void *args) {
	reinterpret_cast<RX*>(args)->audio_dma_read_task();
}

RX::RX(i2s_port_t rx_port) :
		array_read_cnt(0),
		audio_buffer(), buffer_fill_pos(0), buffer_read_pos(0),
		is_running(false), volume_estimate(-40),
		audio_task(nullptr),
		processing_task_handle(nullptr),
		current_dc_value(0),
		i2s_port(rx_port),
		gain(255) {

}

void RX::calculate_audio_rms() {
	uint64_t rms_sum = 0;

	for(auto sample : audio_buffer[buffer_fill_pos]) {
		rms_sum += int32_t(sample) * sample;
	}

	rms_sum /= audio_buffer[buffer_fill_pos].size();

	float rms_value = (sqrt(float(rms_sum)) / INT16_MAX) * sqrt(2);
	if(rms_value <= 0.0001)
		volume_estimate = -40;
	else
		volume_estimate = log10(rms_value) * 10;
}

void RX::audio_dma_read_task() {
	while(true) {
		while(!is_running)
			vTaskDelay(CONFIG_XASAUDIO_RX_FRAMELENGTH / portTICK_PERIOD_MS);

		size_t read_bytes = 0;

		auto & read_buffer = audio_buffer[buffer_fill_pos];

		i2s_read(i2s_port, raw_dma_buffer.data(), raw_dma_buffer.size(), &read_bytes, portMAX_DELAY);
		// ESP_LOGD("Audio RX", "Read %d bytes, expected %d", read_bytes, raw_dma_buffer.size());

		uint8_t *data_ptr = reinterpret_cast<uint8_t *>(raw_dma_buffer.data()) + 0;
		for(int i=0; i < XASAUDIO_RX_FRAME_SAMPLE_NO; i++) {
			int32_t temp = *reinterpret_cast<int32_t*>(data_ptr) / 16384;

			int16_t current_dc_sample = current_dc_value >> 16;
			current_dc_value += (temp - current_dc_sample) * 1024;

			temp -= current_dc_sample;

			temp = (temp * gain) / 255;

			if(temp > INT16_MAX)
				temp = INT16_MAX;
			else if(temp < INT16_MIN)
				temp = INT16_MIN;

			read_buffer[i] = temp;
			data_ptr += 8;
		}

		calculate_audio_rms();

		buffer_fill_pos = (buffer_fill_pos + 1) % audio_buffer.size();
		if(buffer_read_pos == buffer_fill_pos) {
			buffer_read_pos = (buffer_read_pos + 1) % audio_buffer.size();
			ESP_LOGW("Audio RX", "Buffer wasn't read fast enough!");
		}

		if(processing_task_handle != nullptr)
			xTaskNotify(processing_task_handle, 0, eNoAction);
	}
}

void RX::init(TaskHandle_t processing_task, const i2s_pin_config_t &pin_cfg) {
	i2s_config_t cfg = {};

	cfg.mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_RX);
	cfg.sample_rate = CONFIG_XASAUDIO_RX_SAMPLERATE;
	cfg.bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT;
	cfg.channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT;
	cfg.communication_format = I2S_COMM_FORMAT_I2S_MSB;
	cfg.intr_alloc_flags = 0;

	uint32_t num_buffer_bytes = XASAUDIO_RX_FRAME_SAMPLE_NO * 2;

	cfg.dma_buf_count = 1 + ((num_buffer_bytes - 1) / 512);
	cfg.dma_buf_len = 512;

	i2s_driver_install(i2s_port, &cfg, 0, nullptr);
	i2s_set_pin(i2s_port, &pin_cfg);

	processing_task_handle = processing_task;

	xTaskCreate(call_rx_dma_read, "XasAudio RX DMA", 3*1024, this, configMAX_PRIORITIES - 2, &audio_task);
}

bool RX::has_new_audio() {
	if(!is_running)
		return false;
	if(buffer_fill_pos == buffer_read_pos)
		return false;

	return true;
}
const rx_buffer_t &RX::get_buffer() {
	const rx_buffer_t & out_buffer = audio_buffer[buffer_read_pos];

	if(buffer_read_pos != buffer_fill_pos)
		buffer_read_pos = (buffer_read_pos + 1) % audio_buffer.size();

	return out_buffer;
}

void RX::start() {
	if(is_running)
		return;

	is_running = true;

	i2s_start(i2s_port);
}
void RX::stop() {
	if(!is_running)
		return;

	is_running = false;

	i2s_stop(i2s_port);
}

float RX::get_volume_estimate() {
	if(!is_running)
		return -40;

	return volume_estimate;

}
} /* namespace Audio */
} /* namespace Xasin */
