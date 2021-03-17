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

#include <cmath>

#define LOG_LOCAL_LEVEL ESP_LOG_INFO
#include "esp_log.h"

#define U16_TOP int32_t(65536)
#define U16_MAX	(U16_TOP -1)
#define S16_TOP	int32_t(32768)
#define S16_MAX (S16_TOP -1)

namespace Xasin {
namespace Peripheral {

///////////////////////

void start_audio_task(void *data) {
	reinterpret_cast<AudioHandler *>(data)->_audio_task();
}

//////////////////////

AudioSample::AudioSample() : volume(50000) {
}
AudioSample::~AudioSample() {}
int16_t AudioSample::get_chunk() {
	return 0;
}
bool AudioSample::is_done() {
	return true;
}

/////////////////////

SquareWave::SquareWave(uint16_t frequency, uint16_t volume, uint32_t duration) {
	maxDiv = ((44100<<8)/frequency);
	currentDiv = 0;

	cVolume = 0;
	this->volume = fmax(volume, 1);

	this->ticks_left = (duration*44100) / 1000;
}

int16_t SquareWave::get_chunk() {
	currentDiv += 1<<8;
	if(currentDiv > maxDiv)
		currentDiv -= maxDiv;

	if(ticks_left > 0)
		ticks_left--;
	else
		volume = 0;

	if(volume > (cVolume / U16_TOP))
		cVolume += ((int32_t(volume) * S16_TOP) - cVolume/2) / int32_t(1<<9);
	else
		cVolume -= (cVolume/2 - (int32_t(volume) * S16_TOP)) / int32_t(1<<12);

	return (currentDiv > (maxDiv/2) ? 1 : -1) * int32_t(cVolume / 131072);
}

bool SquareWave::is_done() {
	return ((ticks_left == 0) && (cVolume < (2<<24)));
}

SawtoothWave::SawtoothWave(uint16_t frequency, uint16_t volume, uint32_t duration) {
	maxDiv = ((44100<<8)/frequency);
	currentDiv = 0;

	cVolume = 0;
	this->volume = fmax(volume, 1);

	this->ticks_left = (duration*44100) / 1000;
}

int16_t SawtoothWave::get_chunk() {
	currentDiv += 256;
	if(currentDiv > maxDiv)
		currentDiv -= maxDiv;

	if(ticks_left > 0)
		ticks_left--;
	else
		volume = 0;

	if(volume > (cVolume / U16_TOP))
		cVolume += ((int32_t(volume) * S16_TOP) - cVolume/2) / int32_t(1<<9);
	else
		cVolume -= (cVolume/2 - (int32_t(volume) * S16_TOP)) / int32_t(1<<12);

	return ((currentDiv<<7) / maxDiv) * (cVolume >> 24);
}

bool SawtoothWave::is_done() {
	return ((ticks_left == 0) && (cVolume < (5<<24)));
}

TriangleWave::TriangleWave(uint16_t frequency, uint16_t volume, uint32_t duration) {
	maxDiv = ((44100<<8)/frequency);
	currentDiv = 0;

	cVolume = 0;
	this->volume = fmax(volume, 1);

	this->ticks_left = (duration*44100) / 1000;
}

int16_t TriangleWave::get_chunk() {
	currentDiv += 256;
	if(currentDiv > maxDiv)
		currentDiv -= maxDiv;

	if(ticks_left > 0)
		ticks_left--;
	else
		volume = 0;

	if(volume > (cVolume / U16_TOP))
		cVolume += ((int32_t(volume) * S16_TOP) - cVolume/2) / int32_t(1<<9);
	else
		cVolume -= (cVolume/2 - (int32_t(volume) * S16_TOP)) / int32_t(1<<12);

	uint16_t bufSlope = ((currentDiv<<16) / maxDiv);

	if(bufSlope>>15)
		return ((bufSlope>>8) - (1<<8) + 1) * (cVolume>>24);
	else
		return (((1<<8)-2) - (bufSlope>>8)) * (cVolume>>24);
}

bool TriangleWave::is_done() {
	return ((ticks_left == 0) && (cVolume < (5<<24)));
}

////////////////////

AudioCassette::AudioCassette(const uint8_t *start, size_t length, uint16_t volume, uint8_t sample_prescaling)
	: AudioSample(),
	readStart(start), readHead(readStart), readEnd(readStart + length),
	samp_presc(sample_prescaling), presc_counter(0) {

	this->volume = volume;
}

AudioCassette::AudioCassette(const AudioCassette &top) :
	AudioCassette(top.readStart, top.readEnd - top.readStart, top.volume, top.samp_presc) {
}

AudioCassette::AudioCassette() : AudioSample(),
		readStart(nullptr), readHead(nullptr), readEnd(nullptr), samp_presc(1), presc_counter(0) {
}

int16_t AudioCassette::get_chunk() {
	if((readHead+1) >= readEnd)
		return 0;

	int32_t rData = int32_t(*readHead)     - 127;
	int32_t nData = int32_t(*(readHead+1)) - 127;

	presc_counter++;
	if(presc_counter >= samp_presc) {
		readHead++;
		presc_counter = 0;
	}


	return (((rData*(samp_presc-presc_counter) + nData*(presc_counter))/samp_presc) * int32_t(volume)) / 255;
}
bool AudioCassette::is_done() {
	return (readHead+1) >= readEnd;
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

		xSemaphoreTake(sampleMutex, portMAX_DELAY);
		for(uint16_t i=0; i<audioBuffer.size(); i++) {
			int32_t sampBuff = 0;
			for(auto c : currentSamples)
				sampBuff += c->get_chunk();

			sampBuff = (sampBuff*volumeMod)/255;
			if(sampBuff > ((1<<15)-1))
				audioBuffer[i] = (1<<15)-1;
			else if(sampBuff < -((1<<15)-1))
				audioBuffer[i] = -((1<<15)-1);
			else
				audioBuffer[i] = sampBuff;
		}
		xSemaphoreGive(sampleMutex);

		uint8_t sampI = 0;
		while(sampI < currentSamples.size()) {
			AudioSample *samp = currentSamples.at(sampI);

			if(samp->is_done()) {
				ESP_LOGD("XAudio", "Deleting sample %lu\n", long(samp));
				delete samp;

				currentSamples.erase(currentSamples.begin()+sampI);

				continue;
			}

			sampI++;
		}

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

void AudioHandler::notify_task() {
	xTaskNotify(audioTask, 0, eNoAction);
}

void AudioHandler::get_audio_lock() {
	xSemaphoreTake(sampleMutex, portMAX_DELAY);
}
void AudioHandler::release_audio_lock() {
	xSemaphoreGive(sampleMutex);
}

} /* namespace Peripheral */
} /* namespace Xasin */
