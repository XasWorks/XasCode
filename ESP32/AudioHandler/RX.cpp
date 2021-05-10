/*
 * RX.cpp
 *
 *  Created on: 12 Dec 2020
 *      Author: xasin
 */

#include "xasin/audio/RX.h"

#include <xasin/audio/exact_blackman_lut.h>

#include <cmath>
#include <cstring>

#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE
#include <esp_log.h>

namespace Xasin {
namespace Audio {

constexpr float calculate_exact_blackman(float nN) {
	return 0.426591 - 0.496561*cosf(2*M_PI*nN) + 0.076848*cosf(4*M_PI*nN);
}

float get_blackman(float nN) {
	nN *= 200;
	int lut_n = floorf(nN);
	float lut_p = nN - lut_n;

	return XNM::Audio::blackman_lut[lut_n] * (1.0F - lut_p) + XNM::Audio::blackman_lut[lut_n+1] * lut_p;
}

void call_rx_dma_read(void *args) {
	reinterpret_cast<RX*>(args)->audio_dma_read_task();
}

RX::RX(uint8_t data_offset, i2s_port_t rx_port) :
		data_offset(data_offset),
		array_read_cnt(0),
		audio_buffer(), buffer_fill_pos(0), buffer_read_pos(0),
		is_running(false), amplitude(0),
		audio_task(nullptr),
		processing_task_handle(nullptr),
		current_dc_value(0),
		i2s_port(rx_port),
		gain(255) {

}

void RX::calculate_audio_rms() {
	uint64_t rms_sum = 0;

	for(auto sample : audio_buffer[buffer_fill_pos]) {
		rms_sum += int64_t(sample) * sample;
	}

	rms_sum /= audio_buffer[buffer_fill_pos].size();

	amplitude = (sqrt(float(rms_sum)) / INT16_MAX) * sqrt(2);
}

#define LOW1_FRAC 25
#define LOW2_FRAC 50

#define HIGH1_FRAC 150
#define HIGH2_FRAC 150

void RX::audio_dma_read_task() {
	int64_t lowpass_1 = 0;
	int64_t lowpass_2 = 0;

	int64_t highpass_1 = 0;
	int64_t highpass_2 = 0;

	while(true) {
		while(!is_running)
			vTaskDelay(CONFIG_XASAUDIO_RX_FRAMELENGTH / portTICK_PERIOD_MS);

		size_t read_bytes = 0;

		auto & read_buffer = audio_buffer[buffer_fill_pos];

		i2s_read(i2s_port, raw_dma_buffer.data(), raw_dma_buffer.size(), &read_bytes, portMAX_DELAY);
		// ESP_LOGD("Audio RX", "Read %d bytes, expected %d", read_bytes, raw_dma_buffer.size());

		// memcpy(read_buffer.data(), raw_dma_buffer.data(), 20);

		uint8_t *data_ptr = reinterpret_cast<uint8_t *>(raw_dma_buffer.data()) + data_offset;
		for(int i=0; i < XASAUDIO_RX_FRAME_SAMPLE_NO; i++) {
			// Input data is in the upper 24 bit, divide by 256 to get a real 24 bit value
			int64_t temp = *reinterpret_cast<int32_t*>(data_ptr) / 256;

			highpass_1 = (highpass_1 * (256 - HIGH1_FRAC) + temp * (HIGH1_FRAC)) / 256;
			highpass_2 = (highpass_2 * (256 - HIGH2_FRAC) + highpass_1 * (HIGH2_FRAC)) / 256;

			lowpass_1 = (highpass_2 * (256 - LOW1_FRAC) + lowpass_1 * (LOW1_FRAC)) / 256;
			lowpass_2 = (lowpass_1  * (256 - LOW2_FRAC) + lowpass_2 * (LOW2_FRAC)) / 256;

			temp = highpass_2 - lowpass_2;

			temp = (temp * gain) / (255);

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

float RX::get_amplitude_rms() {
	return amplitude;
}

float RX::get_volume_estimate() {
	if(!is_running)
		return -40;

	if(amplitude <= 0.0001)
		return -40;
	else
		return log10(amplitude) * 10;
}

float RX::get_goertzel(float frequency, int n_samples) {
	if(n_samples < 0)
		n_samples = 3*XASAUDIO_RX_FRAME_SAMPLE_NO;

	n_samples = CONFIG_XASAUDIO_RX_SAMPLERATE/frequency * floorf(n_samples / (CONFIG_XASAUDIO_RX_SAMPLERATE / frequency));

	const float k = floorf(0.5F + n_samples * frequency / (CONFIG_XASAUDIO_RX_SAMPLERATE));
	const float w = (2 * M_PI * k / n_samples);

	const float cosine_fact = cosf(w);
	const float sine_fact = sinf(w);

	const float coeff = 2 * cosine_fact;

	float q0 = 0;
	float q1 = 0;
	float q2 = 0;

	for(int i=0; i<n_samples; i++) {
		float samp = audio_buffer[(buffer_fill_pos + 1 + i/XASAUDIO_RX_FRAME_SAMPLE_NO) & 0b11][i % XASAUDIO_RX_FRAME_SAMPLE_NO];

		samp *= get_blackman(float(i)/n_samples);

		q0 = coeff * q1 - q2 + samp;
		q2 = q1;
		q1 = q0;
	}

	return sqrtf(powf(q1 - q2 * cosine_fact, 2) + powf(q2 * sine_fact, 2)) / (n_samples * INT16_MAX);
}

int32_t RX::get_autocorrelation(float frequency) {
	float stepsize = 0;
	if(frequency > 0.001/float(CONFIG_XASAUDIO_RX_FRAMELENGTH*3))
		stepsize = CONFIG_XASAUDIO_RX_SAMPLERATE / frequency;
	
	int64_t sum_count = 0;

	uint32_t i = 0;

	for(int i = 0; i <= floorf(stepsize); i++) {
		const auto samp_a = audio_buffer[(buffer_fill_pos + 1 + i/XASAUDIO_RX_FRAME_SAMPLE_NO) & 0b11][i % XASAUDIO_RX_FRAME_SAMPLE_NO];
		
		for(float step = i + stepsize; step < XASAUDIO_RX_FRAME_SAMPLE_NO*3; step += stepsize) {
			const auto samp_b = audio_buffer[(buffer_fill_pos + 1 + int(step)/XASAUDIO_RX_FRAME_SAMPLE_NO) & 0b11][int(step) % XASAUDIO_RX_FRAME_SAMPLE_NO];
		
			sum_count += samp_a * int32_t(samp_b);
		}
	}

	return sum_count / INT16_MAX;
}

} /* namespace Audio */
} /* namespace Xasin */
