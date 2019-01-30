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

namespace Xasin {
namespace Peripheral {

void start_audio_task(void *data) {
	reinterpret_cast<AudioHandler *>(data)->_audio_task();
}

AudioCassette::AudioCassette(const uint8_t *start, size_t length)
	: readStart(start), readHead(readStart), readEnd(readStart + length) {
}

AudioCassette::AudioCassette(const AudioCassette &top) :
	AudioCassette(top.readStart, top.readEnd - top.readStart) {
}

AudioCassette::AudioCassette() : readStart(nullptr), readHead(nullptr), readEnd(nullptr) {
}

int16_t AudioCassette::get_chunk() {
	if(readHead >= readEnd)
		return 0;

	int16_t rData = *readHead;
	readHead++;

	return (rData - 127)*40;
}
bool AudioCassette::is_done() {
	return readHead >= readEnd;
}

AudioHandler::AudioHandler(int samplerate, i2s_port_t i2s_port)
	: audioTask(nullptr),
	  currentCassettes(),
	  samplerate(samplerate), i2s_port(i2s_port) {

}

void AudioHandler::_audio_task() {
	std::array<int16_t, 512> audioBuffer;

	while(true) {
		while(currentCassettes.empty())
			xTaskNotifyWait(0, 0, nullptr, portMAX_DELAY);

		audioBuffer.fill(0);
		for(uint16_t i=0; i<audioBuffer.size(); i++) {
			for(auto &c : currentCassettes)
				audioBuffer[i] += c.get_chunk();
		}

		size_t written_samples = 0;
		i2s_write(i2s_port, audioBuffer.data(), 1024, &written_samples, portMAX_DELAY);


		for(auto i=currentCassettes.begin(); i<currentCassettes.end(); i++) {
			if(i->is_done())
				currentCassettes.erase(i);
		}

		if(currentCassettes.empty())
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

void AudioHandler::insert_cassette(const AudioCassette &cassette) {
	if(audioTask == nullptr)
		return;

	currentCassettes.push_back(cassette);

	xTaskNotify(audioTask, 0, eNoAction);
}

void AudioHandler::insert_cassette(const CassetteCollection &cassettes) {
	if(cassettes.size() == 0)
		return;
	insert_cassette(cassettes.at(esp_random()%cassettes.size()));
}

} /* namespace Peripheral */
} /* namespace Xasin */
