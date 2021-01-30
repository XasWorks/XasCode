/*
 * OpusCassette.h
 *
 *  Created on: 20 Nov 2020
 *      Author: xasin
 */

#ifndef ESP32_AUDIOHANDLER_OPUSCASETTE_H_
#define ESP32_AUDIOHANDLER_OPUSCASETTE_H_

#include <xasin/audio/Source.h>

#include <opus.h>

#include <vector>

#include <stdint.h>

namespace Xasin {
namespace Audio {

// Definition for a constant-bitrate OPUS packet
// embedded into flash (as const character array)
struct opus_audio_bundle_t {
	uint16_t packetsize;
	uint32_t num_packets;
	uint8_t volume;
	const uint8_t * data;
};

typedef std::vector<opus_audio_bundle_t> OpusCassetteCollection;

class OpusCassette : public Source {
private:
	const opus_audio_bundle_t &audio_data;
	uint32_t packet_no;

	OpusDecoder * decoder;

	uint16_t volume;

	// When set, this will cause the volume to gradually
	// lower until the audio is no longer audible, at which
	// point the cassette will deactivate.
	// Fading out should usually happen over approx. 200ms
	bool fade_out_active;

	bool process_frame();

public:
	// When set, will cause this audio to loop, instead of just ending.
	bool repeat;

	OpusCassette(TX &handler, const opus_audio_bundle_t &data);
	~OpusCassette();

	bool is_finished();

	TickType_t remaining_runtime();

	void fade_out();
};

}
}

#endif
