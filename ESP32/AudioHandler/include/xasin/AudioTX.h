

#ifndef XASIN_AUDIO_TX_H
#define XASIN_AUDIO_TX_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/i2s.h"

#include <array>
#include <vector>

namespace Xasin {
namespace Audio {

class AudioSource;

class TX {
public:
	enum audio_tx_state_t {
		STOPPED,			// Threads have not been initialized yet
		RUNNING,			// The audio handler is active and playing
		PROCESSING,		// The audio handler is waiting on processed data
		PROCESSED,		// The processing task finished, data shall be written to DMA
		IDLE,				// There is no audio to be played from any source
	};

private:
	bool clip_detect;

	TaskHandle_t audio_task; // Handle to the small DMA fill task.
	TaskHandle_t processing_task; // Handle to a large-stack processing task,
		// useful to decode Opus audio frames.

	std::vector<AudioSource *> audio_sources;
	SemaphoreHandle_t audio_config_mutex;

	uint16_t volume_estimate;

protected:
	void boop_thread;
	audio_tx_state_t state;

	// This buffer represents the 2x 20ms samples at 48kHz, 16 bit,
	// and is used to exchange data between the DMA buffer and the processing
	// task. 20ms is the length of a regular Opus frame decode.
	// The data here represents the interleaved left/right audio channels
	std::array<int16_t, 1920> audio_buffer;

	// These functions will always add exactly one frame (20ms) of the given data
	// buffer to the audio buffer. volume can be used to reduce the volume
	// of the given sample (independently of the global volume).
	// This function handles the left/right interleaved nature and avoids
	// saturation.
	void add_interleaved_frame(const int16_t *data, uint8_t volume = 255);
	void add_lr_frame(const int16_t *data, bool left = true, uint8_t volume = 255);

public:
	const i2s_port_t i2s_port;
	const int samplerate;

	bool calculate_volume;
	uint16_t volumeMod;

	void audio_dma_fill_task();

	bool largestack_process();

	AudioHandler(int samplerate = 48000, i2i2s_port_t = I2S_NUM_0);

	void init(TaskHandle_t processing_task, const i2s_pin_config_t &pin_config);

	uint16_t get_volume_estimate();
	bool had_clipping();
};

}
}

#endif
