/*
 * BitCasette.cpp
 *
 *  Created on: 20 Nov 2020
 *      Author: xasin
 */

#include <xasin/audio/AudioTX.h>
#include <xasin/audio/ByteCassette.h>

#define LOG_LOCAL_LEVEL ESP_LOG_INFO
#include <esp_log.h>

namespace Xasin {
namespace Audio {

ByteCassette::ByteCassette(TX &audio_handler,
		const uint8_t *sample_ptr, const uint8_t *samp_end, uint32_t samprate)
	: Source(audio_handler),
	  sample_end(samp_end),
	  per_sample_increase((samprate << 16)/CONFIG_XASAUDIO_TX_SAMPLERATE),
	  data_samplerate(samprate) {

	current_sample = sample_ptr;
	sample_position_counter = 0;

	volume = 255;
}

ByteCassette::ByteCassette(TX &handler, const bytecassette_data_t &cassette)
	: ByteCassette(handler, cassette.data_start, cassette.data_end,
			cassette.data_samplerate) {

	if(cassette.volume != 0)
		volume = cassette.volume;
}

ByteCassette::~ByteCassette() {
}

bool ByteCassette::process_frame() {
	if(current_sample == nullptr)
		return false;

	std::array<int16_t, XASAUDIO_TX_FRAME_SAMPLE_NO> temp_buffer = {};

	for(int i=0; i<XASAUDIO_TX_FRAME_SAMPLE_NO; i++) {
		if((current_sample+1) >= sample_end) {
			current_sample = nullptr;
			break;
		}

		int32_t prev_sample = int32_t(*current_sample) - 0x80;
		int32_t next_sample = int32_t(*(current_sample+1)) - 0x80;

		sample_position_counter += per_sample_increase;

		uint16_t sample_fraction = (sample_position_counter & 0xFFFF);

		temp_buffer[i] = (((0xFFFF - sample_fraction) * prev_sample) + (sample_fraction * next_sample)) >> 8;

		current_sample += sample_position_counter >> 16;
		sample_position_counter &= 0xFFFF;
	}

	add_lr_frame(temp_buffer.data(), true, volume);

	return true;
}

void ByteCassette::play(TX &handler, const bytecassette_data_t &cassette) {
	auto temp = new ByteCassette(handler, cassette);
	temp->start(true);

	ESP_LOGD("Audio", "Newly created source is %p", temp);
}

void ByteCassette::play(TX &handler, const ByteCassetteCollection &cassettes) {
	if(cassettes.size() == 0)
		return;

	play(handler, cassettes.at(esp_random()%cassettes.size()));
}

template<>
Source * TX::play(const bytecassette_data_t &sample, bool auto_delete) {
	auto new_sound = new ByteCassette(*this, sample);
	new_sound->start(auto_delete);

	return new_sound;
}

bool ByteCassette::is_finished() {
	return current_sample == nullptr;
}

} /* namespace Audio */
} /* namespace Xasin */
