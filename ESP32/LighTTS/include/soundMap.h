

#ifndef LIGHTTS_SOUNDMAP_H
#define LIGHTTS_SOUNDMAP_H

#include "AudioHandler.h"

#include <map>
#include <string>

#define LIGHTTS_PRESCALING_FACTOR 8

namespace Xasin {
namespace LighTTS {

struct audio_samp_t {
	const uint8_t *audioStart;
	const size_t   length;
};

extern Peripheral::AudioHandler *audioHandler;
extern const std::map<std::string, audio_samp_t> lightts_available_samples;

const Xasin::Peripheral::AudioCassette get_sample_for(std::string key);

void speak_string(std::string text);

}
}

#endif
