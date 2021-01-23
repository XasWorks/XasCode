/*
 * OpusCassette.cpp
 *
 *  Created on: 27 Dec 2020
 *      Author: Xasin, Neira
 */

#include <xasin/audio/OpusCassette.h>
#include <xasin/audio/AudioTX.h>

#include <xasin/audio/ByteCassette.h>

#include <array>

namespace Xasin {
namespace Audio {

OpusCassette::OpusCassette(TX &handler, const opus_audio_bundle_t &data) :
	Source(handler),
	audio_data(data), packet_no(0),
	decoder(nullptr), volume(data.volume),
	fade_out_active(false), repeat(false) {

	decoder = opus_decoder_create(CONFIG_XASAUDIO_TX_SAMPLERATE, 1, nullptr);
}

OpusCassette::~OpusCassette() {
	if(decoder != nullptr)
		opus_decoder_destroy(decoder);
}

bool OpusCassette::process_frame() {
	if(decoder == nullptr)
		return false;

	if(packet_no >= audio_data.num_packets)
		return false;

	if(volume == 0)
		return false;

	std::array<int16_t, XASAUDIO_TX_FRAME_SAMPLE_NO> decode_buffer = {};

	const uint8_t * data_ptr = audio_data.data;
	data_ptr += packet_no * audio_data.packetsize;

	opus_decode(decoder, data_ptr, audio_data.packetsize,
		decode_buffer.data(), decode_buffer.size(), 0);

	add_lr_frame(decode_buffer.data(), true, volume);

	packet_no++;
	if(repeat && (packet_no >= audio_data.num_packets))
		packet_no = 0;

	if(fade_out_active)
		volume *= 0.6F;

	return true;
}

bool OpusCassette::is_finished() {
	if((volume == 0) && fade_out_active)
		return true;
	if(packet_no >= audio_data.num_packets)
		return true;
	if(decoder == nullptr)
		return true;

	return false;
}

void OpusCassette::fade_out() {
	fade_out_active = true;
}

template<>
Source * TX::play(const opus_audio_bundle_t &data, bool auto_delete) {
	// If this is the case, it is assumed this is a raw byte cassette,
	// and will be played as such. This is meant for compatibility with
	// raw formats.
	if(data.packetsize <= 1) {
		bytecassette_data_t byte_cassette = {
			data.data,
			data.data + data.num_packets,
			44100,
			data.volume
		};

		return this->play(byte_cassette, auto_delete);
	}

	auto new_cassette = new OpusCassette(*this, data);
	new_cassette->start(auto_delete);

	return new_cassette;
}

template<>
Source * TX::play(const OpusCassetteCollection &collection, bool auto_delete) {
	if(collection.size() == 0)
		return nullptr;

	return play(collection.at(esp_random()%collection.size()), auto_delete);
}

}
}
