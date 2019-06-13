/*
 * AudioHandler.cpp
 *
 *  Created on: 20 Jan 2019
 *      Author: xasin
 */

#include <cstring>
#include <array>

#include "AudioHandler.h"

#include "driver/gpio.h"

#define LOG_LOCAL_LEVEL ESP_LOG_INFO
#include "esp_log.h"

namespace Xasin {
namespace Peripheral {

///////////////////////

void start_audio_task(void *data) {
	reinterpret_cast<AudioHandler *>(data)->_audio_task();
}

//////////////////////

AudioSample::AudioSample() : volume(255) {
}
AudioSample::~AudioSample() {}
int16_t AudioSample::get_chunk() {
	return 0;
}
bool AudioSample::is_done() {
	return true;
}

/////////////////////

SquareWave::SquareWave(uint16_t frequency, uint8_t volume, uint32_t duration) : maxDiv(44100/frequency) {
	currentDiv = 0;

	this->volume = volume;

	this->ticks_left = (duration*44100) / 1000;
}

int16_t SquareWave::get_chunk() {
	currentDiv++;
	if(currentDiv > maxDiv)
		currentDiv = 0;

	if(ticks_left > 0)
		ticks_left--;

	return (currentDiv > (maxDiv/2) ? 1 : -1) * volume * 255;
}

bool SquareWave::is_done() {
	return ticks_left == 0;
}

////////////////////

AudioCassette::AudioCassette(const uint8_t *start, size_t length, uint8_t volume)
	: AudioSample(),
	readStart(start), readHead(readStart), readEnd(readStart + length) {
}

AudioCassette::AudioCassette(const AudioCassette &top) :
	AudioCassette(top.readStart, top.readEnd - top.readStart, top.volume) {
}

AudioCassette::AudioCassette() : AudioSample(),
		readStart(nullptr), readHead(nullptr), readEnd(nullptr) {
}

int16_t AudioCassette::get_chunk() {
	if(readHead >= readEnd)
		return 0;

	int16_t rData = *readHead;
	readHead++;

	return (rData - 127)*volume;
}
bool AudioCassette::is_done() {
	return readHead >= readEnd;
}

AudioHandler::AudioHandler(int samplerate, i2s_port_t i2s_port)
	: audioTask(nullptr),
	  currentSamples(),
	  samplerate(samplerate), i2s_port(i2s_port),
	  volumeMod(255) {

	sampleMutex = xSemaphoreCreateMutex();
}

void AudioHandler::_audio_task() {
	std::array<int16_t, 512> audioBuffer;

	ESP_LOGI("XAudio", "Audio task started!");

	while(true) {
		while(currentSamples.empty())
			xTaskNotifyWait(0, 0, nullptr, portMAX_DELAY);

		audioBuffer.fill(0);
		xSemaphoreTake(sampleMutex, portMAX_DELAY);
		for(uint16_t i=0; i<audioBuffer.size(); i++) {
			for(auto c : currentSamples)
				audioBuffer[i] += (int32_t(c->get_chunk())*volumeMod)/255;
		}
		for(auto i=currentSamples.begin(); i<currentSamples.end(); i++) {
			AudioSample *samp = *i;

			if(samp->is_done()) {
				ESP_LOGD("XAudio", "Deleting sample %lu\n", long(samp));
				delete samp;
				currentSamples.erase(i);
			}
		}
		xSemaphoreGive(sampleMutex);

		size_t written_samples = 0;
		i2s_write(i2s_port, audioBuffer.data(), 1024, &written_samples, portMAX_DELAY);

		if(currentSamples.empty())
			i2s_zero_dma_buffer(i2s_port);
	}
}

void AudioHandler::start_thread(const i2s_pin_config_t &pinCFG) {
	i2s_config_t cfg = {};
	memset(&cfg, 0, sizeof(cfg));

	cfg.mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_TX);
	cfg.sample_rate = samplerate;
	cfg.bits_per_sample = i2s_bits_per_sample_t(16);
	cfg.channel_format = I2S_CHANNEL_FMT_ONLY_LEFT;
	cfg.communication_format = I2S_COMM_FORMAT_I2S_MSB;
	cfg.intr_alloc_flags = 0;
	cfg.dma_buf_count = 2;
	cfg.dma_buf_len = 1024;

	i2s_driver_install(i2s_port, &cfg, 0, nullptr);
	i2s_set_pin(i2s_port, &pinCFG);

	xTaskCreatePinnedToCore(start_audio_task, "Audio", 5*1024, this, configMAX_PRIORITIES - 1, &audioTask, 1);
}

void AudioHandler::insert_sample(AudioSample *sample) {
	if(audioTask == nullptr)
		return;

	ESP_LOGD("XAudio", "Adding sample %lu\n", long(sample));

	xSemaphoreTake(sampleMutex, portMAX_DELAY);
	currentSamples.push_back(sample);
	xSemaphoreGive(sampleMutex);

	xTaskNotify(audioTask, 0, eNoAction);
}

void AudioHandler::insert_cassette(const AudioCassette &top) {
	insert_sample(new AudioCassette(top));
}
void AudioHandler::insert_cassette(const CassetteCollection &cassettes) {
	if(cassettes.size() == 0)
		return;

	insert_cassette(cassettes.at(esp_random()%cassettes.size()));
}
} /* namespace Peripheral */
} /* namespace Xasin */
