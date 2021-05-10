/*
 * Source.h
 *
 *  Created on: 20 Nov 2020
 *      Author: xasin
 */

#ifndef ESP32_AUDIOHANDLER_SOURCE_H_
#define ESP32_AUDIOHANDLER_SOURCE_H_

#include <stdint.h>

#include <freertos/FreeRTOS.h>

namespace Xasin {
namespace Audio {

class TX;

class Source {
private:
	bool was_started;
	bool is_deletable;

protected:
friend TX;

	TX &audio_handler;

	virtual bool process_frame();
	void boop_playback();

	// These functions will always add exactly one frame (20ms) of the given data
	// buffer to the audio buffer. volume can be used to reduce the volume
	// of the given sample (independently of the global volume).
	// This function handles the left/right interleaved nature and avoids
	// saturation.
	void add_interleaved_frame(const int16_t *data, uint8_t volume = 255);
	void add_lr_frame(const int16_t *data, bool left = true, uint8_t volume = 255);

public:
	Source(TX &handler);
	virtual ~Source();

	bool can_be_deleted();

	virtual bool is_finished();
	virtual bool has_audio();

	virtual TickType_t remaining_runtime();

	virtual void fade_out();

	void start(bool deletable = true);
	void release();
};

} /* namespace Audio */
} /* namespace Xasin */

#endif /* ESP32_AUDIOHANDLER_SOURCE_H_ */
