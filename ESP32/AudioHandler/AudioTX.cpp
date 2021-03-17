
#include "xasin/audio/AudioTX.h"
#include "xasin/audio/Source.h"

#include <cstring>
#include <cmath>

#define LOG_LOCAL_LEVEL ESP_LOG_INFO
#include "esp_log.h"

namespace Xasin {
namespace Audio {

void start_audio_task(void *arg) {
	reinterpret_cast<TX *>(arg)->audio_dma_fill_task();
}

TX::TX(i2s_port_t port) :
		audio_sources(), i2s_port(port) {

	audio_task = nullptr;
	processing_task = nullptr;

	audio_config_mutex = xSemaphoreCreateMutex();

	volume_estimate = 0;
	clipping = false;

	state = IDLE;

	calculate_volume = false;
	volume_mod = 255;
}

void TX::calculate_audio_rms() {
	uint64_t rms_sum = 0;

	for(int16_t sample : audio_buffer) {
		rms_sum += int32_t(sample) * sample;
	}

	rms_sum /= audio_buffer.size();

	float rms_value = (sqrt(float(rms_sum)) / INT16_MAX) * sqrt(2);
	if(rms_value <= 0.0001)
		volume_estimate = -40;
	else
		volume_estimate = log10(rms_value) * 10;
}

void TX::boop_thread() {
	if(audio_task == nullptr)
		return;

	xTaskNotify(audio_task, 0, eNoAction);
}

void TX::remove_source(Source * source) {
	xSemaphoreTake(audio_config_mutex, portMAX_DELAY);

	ESP_LOGD("XasAudio", "Erasing source 0x%p", source);

	for(auto i = audio_sources.begin(); i != audio_sources.end();) {
		if(*i == source)
			i = audio_sources.erase(i);
		else
			i++;
	}

	ESP_LOGD("XasAudio", "New held count: %d", audio_sources.size());

	xSemaphoreGive(audio_config_mutex);
}

void TX::insert_source(Source * source) {
	xSemaphoreTake(audio_config_mutex, portMAX_DELAY);

	ESP_LOGD("XasAudio", "Adding source 0x%p", source);

	audio_sources.push_back(source);

	ESP_LOGD("XasAudio", "New held count: %d", audio_sources.size());
	xSemaphoreGive(audio_config_mutex);

	xTaskNotify(audio_task, 0, eNoAction);
}

void TX::add_interleaved_frame(const int16_t *data, uint8_t volume) {
	for(int i=0; i < 2*XASAUDIO_TX_FRAME_SAMPLE_NO; i++) {
		int32_t new_audio_value = audio_buffer[i];
		new_audio_value += (volume_mod * volume * int32_t(*data)) >> 16;

		if(new_audio_value > INT16_MAX) {
			clipping = true;
			new_audio_value = INT16_MAX;
		}
		else if(new_audio_value < INT16_MIN) {
			clipping = true;
			new_audio_value = INT16_MIN;
		}

		audio_buffer[i] = int16_t(new_audio_value);

		data++;
	}
}

void TX::add_lr_frame(const int16_t *data, bool left, uint8_t volume) {
	for(int i=0; i < XASAUDIO_TX_FRAME_SAMPLE_NO; i++) {
		int audio_index = i*2 + (left ? 1 : 0);

		int32_t new_audio_value = audio_buffer[audio_index];
		new_audio_value += (volume_mod * volume * int32_t(*data)) >> 16;

		if(new_audio_value > INT16_MAX) {
			clipping = true;
			new_audio_value = INT16_MAX;
		}
		else if(new_audio_value < INT16_MIN) {
			clipping = true;
			new_audio_value = INT16_MIN;
		}

		audio_buffer[audio_index] = int16_t(new_audio_value);

		data++;
	}
}

void TX::audio_dma_fill_task() {
	int audio_idle_count = 0;

	while(true) {
		// As long as we haven't been idling for a while, continue playback.
		if(audio_idle_count < 0) {
			uint32_t written_data = 0;
			i2s_write(i2s_port, audio_buffer.data(), audio_buffer.size()*2, &written_data, portMAX_DELAY);
		}
		// The I2S Port has been stopped, simply wait for new data, a new audio source or similar
		else {
			state = IDLE;
			xTaskNotifyWait(0, 0, nullptr, portMAX_DELAY);
		}

		// First things first we need to call the large processing thread
		// to see what needs to be done.
		memset(audio_buffer.data(), 0, audio_buffer.size()*2);
		state = PROCESSING;
		xTaskNotify(processing_task, 0, eNoAction);
		while(state == PROCESSING)
			xTaskNotifyWait(0, 0, nullptr, portMAX_DELAY);
		ESP_LOGV("XasAudio", "Processing finished.");

		// An audio source reported it has some audio to play
		if(state == RUNNING) {
			// We had put the i2s unit to sleep, unpause it.
			if(audio_idle_count == 0) {
				ESP_LOGD("XasAudio", "Transitioning to running.");
				i2s_start(i2s_port);
			}

			// Reset the idle counter.
			audio_idle_count = -10;
		}
		// No more audio was added, increment idle counter
		// We can't immediately stop playing as the DMA buffer must be emptied.
		else if(audio_idle_count < 0) {
			audio_idle_count++;

			// We reached our four block idle time, stop the I2S Clock
			// This helps conserve energy and processing power
			if(audio_idle_count == 0) {
				ESP_LOGD("XasAudio", "Transitioning to idle.");
				i2s_stop(i2s_port);
			}
		}
	}
}

bool TX::largestack_process() {
	if(state != PROCESSING)
		return false;

	ESP_LOGV("XasAudio", "Processing next batch.");

	// We need to get the FreeRTOS Mutex to prevent concurrent modification
	// of the audio sources list.
	xSemaphoreTake(audio_config_mutex, portMAX_DELAY);
	std::vector<Source *> sources_copy = audio_sources;
	xSemaphoreGive(audio_config_mutex);

	bool source_is_playing = false;
	for(auto source : sources_copy) {
		source_is_playing |= source->process_frame();
	}

	if(calculate_volume)
		calculate_audio_rms();
	else
		volume_estimate = 0;

	if(source_is_playing)
		state = RUNNING;
	else
		state = PRE_IDLE;

	xTaskNotify(audio_task, 0, eNoAction);

	for (auto source : sources_copy)
	{
		// FIXME This really should be a std::shared_pointer, it's a perfect
		// use case, I just need to muster the courage to use it :P
		if (source->is_finished() && source->can_be_deleted())
		{
			ESP_LOGD("XasAudio", "Starting delete of 0x%p", source);
			delete source;
		}
	}

	return true;
}

void TX::init(TaskHandle_t processing_task, const i2s_pin_config_t &pin_config) {
	i2s_config_t cfg = {};

	cfg.mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_TX);
	cfg.sample_rate = CONFIG_XASAUDIO_TX_SAMPLERATE;
	cfg.bits_per_sample = i2s_bits_per_sample_t(16);
	cfg.channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT;
	cfg.communication_format = I2S_COMM_FORMAT_I2S_MSB;
	cfg.intr_alloc_flags = 0;

	uint32_t num_buffer_bytes = XASAUDIO_TX_FRAME_SAMPLE_NO * 2 * CONFIG_XASAUDIO_TX_DMA_COUNT;

	cfg.dma_buf_count = 1 + (num_buffer_bytes / 1024);
	cfg.dma_buf_len = 1024;

	i2s_driver_install(i2s_port, &cfg, 0, nullptr);
	i2s_set_pin(i2s_port, &pin_config);

	this->processing_task = processing_task;

	xTaskCreate(start_audio_task, "XasAudio TX DMA", 3*1024, this, 7, &audio_task);
}

float TX::get_volume_estimate() {
	return volume_estimate;
}

}
}
