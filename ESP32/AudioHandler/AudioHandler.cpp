/*
 * AudioHandler.cpp
 *
 *  Created on: 20 Jan 2019
 *      Author: xasin
 */

#include "AudioHandler.h"
#include <cstring>

namespace Xasin {
namespace Peripheral {

void start_audio_task(void *data) {
	reinterpret_cast<AudioHandler *>(data)->_audio_task();
}

AudioCassette::AudioCassette(const uint8_t *start, size_t length)
	: readStart(start), readHead(readStart), readEnd(readStart + length) {
}

AudioHandler::AudioHandler(int samplerate, i2s_port_t i2s_port, i2s_dac_mode_t dac_mode)
	: audioTask(nullptr),
	  currentCassette(nullptr),
	  samplerate(samplerate), i2s_port(i2s_port), dac_mode(dac_mode) {

}

void AudioHandler::_audio_task() {
	while(true) {
		while(currentCassette == nullptr)
			xTaskNotifyWait(0, 0, nullptr, portMAX_DELAY);

		int samples_to_write = currentCassette->readEnd - currentCassette->readHead;
		if(samples_to_write > 512)
			samples_to_write = 512;

		size_t written_samples = 0;
		i2s_write_expand(i2s_port, currentCassette->readHead, samples_to_write,
				8, 16, &written_samples, portMAX_DELAY);

		currentCassette->readHead += written_samples;
		if(currentCassette->readHead >= currentCassette->readEnd)
			currentCassette = nullptr;
	}
}

void AudioHandler::start_thread() {
	i2s_config_t cfg = {};
	memset(&cfg, 0, sizeof(cfg));

	cfg.mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_TX | I2S_MODE_DAC_BUILT_IN);
	cfg.sample_rate = samplerate;
	cfg.bits_per_sample = i2s_bits_per_sample_t(16);
	cfg.channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT;
	cfg.communication_format = I2S_COMM_FORMAT_I2S_MSB;
	cfg.intr_alloc_flags = 0;
	cfg.dma_buf_count = 2;
	cfg.dma_buf_len = 1024;

	i2s_driver_install(i2s_port, &cfg, 0, nullptr);
	i2s_set_dac_mode(dac_mode);

	xTaskCreatePinnedToCore(start_audio_task, "Audio", 4*1024, this, configMAX_PRIORITIES - 1, &audioTask, 1);
}

void AudioHandler::insert_cassette(AudioCassette &cassette) {
	if(audioTask == nullptr)
		return;

	cassette.readHead = cassette.readStart;
	currentCassette = &cassette;

	xTaskNotify(audioTask, 0, eNoAction);
}

} /* namespace Peripheral */
} /* namespace Xasin */
