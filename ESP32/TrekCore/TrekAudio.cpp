/*
 * TrekAudio.cpp
 *
 *  Created on: 27 Feb 2019
 *      Author: xasin
 */


#include "xasin/TrekAudio.h"

#include "audioHeaders/input_error.h"
#include "audioHeaders/input_ok.h"
#include "audioHeaders/input_requested.h"
#include "audioHeaders/keypress.h"
#include "audioHeaders/major_error.h"
#include "audioHeaders/program_busy.h"
#include "audioHeaders/program_failed.h"
#include "audioHeaders/program_finished.h"

namespace Xasin {
namespace Trek {

typedef const Audio::bytecassette_data_t snippet;
#define SNIP_DEF(name) snippet s_ ## name = {name, name + sizeof(name), 44100, 150}
#define SNIP_DEF_VOL(name, volume) snippet s_ ## name = {name, name + sizeof(name), 44100, volume}

#ifdef TREKCORE_AUDIO_ENABLED
snippet s_keypress = { keypress, keypress + sizeof(keypress), 44100, 20 };

SNIP_DEF(input_ok);
SNIP_DEF(input_error);
SNIP_DEF(input_requested);

SNIP_DEF(program_busy);
SNIP_DEF(program_failed);
SNIP_DEF_VOL(program_finished, 170);

SNIP_DEF(major_error);
#endif

Audio::TX *audioHandler = nullptr;

void init(Audio::TX &audio) {
#ifdef TREKCORE_AUDIO_ENABLED
	audioHandler = &audio;
#endif
}


#define SNIP_PLAY(key, name) case key : Audio::ByteCassette::play(*audioHandler, s_ ## name); break
void play(signal_type_t signal) {
#ifdef TREKCORE_AUDIO_ENABLED
	if(audioHandler == nullptr)
		return;

	switch(signal) {
	default: break;
	SNIP_PLAY(KEYPRESS, keypress);

	SNIP_PLAY(INPUT_OK, input_ok);
	SNIP_PLAY(INPUT_BAD, input_error);
	SNIP_PLAY(INPUT_REQ, input_requested);

	SNIP_PLAY(PROG_BUSY, program_busy);
	SNIP_PLAY(PROG_FAILED, program_failed);
	SNIP_PLAY(PROG_DONE, program_finished);

	SNIP_PLAY(ERROR_MAJOR, major_error);

	};
#endif
}

}
}
