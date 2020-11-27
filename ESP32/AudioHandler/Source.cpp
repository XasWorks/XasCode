/*
 * Source.cpp
 *
 *  Created on: 20 Nov 2020
 *      Author: xasin
 */

#include "xasin/audio/Source.h"
#include "xasin/audio/AudioTX.h"

namespace Xasin {
namespace Audio {

Source::Source(TX &handler) : audio_handler(handler) {
	was_started = false;
	is_deletable = false;
}

Source::~Source() {
	audio_handler.remove_source(this);
}

bool Source::process_frame() {
	return false;
}

void Source::boop_playback() {
	audio_handler.boop_thread();
}

void Source::add_interleaved_frame(const int16_t *data, uint8_t volume) {
	audio_handler.add_interleaved_frame(data, volume);
}
void Source::add_lr_frame(const int16_t *data, bool ch_left, uint8_t volume) {
	audio_handler.add_lr_frame(data, ch_left, volume);
}

bool Source::can_be_deleted() {
	return is_deletable;
}

bool Source::is_finished() {
	return true;
}

void Source::start(bool deletable) {
	if(was_started)
		return;

	is_deletable = deletable;

	was_started = true;
	audio_handler.insert_source(this);
}

} /* namespace Audio */
} /* namespace Xasin */
