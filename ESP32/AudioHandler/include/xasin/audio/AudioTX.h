

#ifndef XASIN_AUDIO_TX_H
#define XASIN_AUDIO_TX_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "driver/i2s.h"

#include <memory>

#include <array>
#include <vector>

#define XASAUDIO_TX_FRAME_SAMPLE_NO ((CONFIG_XASAUDIO_TX_SAMPLERATE * CONFIG_XASAUDIO_TX_FRAMELENGTH)/1000)

namespace Xasin {
namespace Audio {

class Source;

class TX {
public:
	enum audio_tx_state_t {
		RUNNING,			// The audio handler is active and playing
		PROCESSING,			// The audio handler is waiting on processed data
		PRE_IDLE,				// There is no audio to be played from any source
		IDLE,
	};

private:
	TaskHandle_t audio_task; // Handle to the small DMA fill task.
	TaskHandle_t processing_task; // Handle to a large-stack processing task,
		// useful to decode Opus audio frames.

	std::vector<Source *> audio_sources;
	SemaphoreHandle_t audio_config_mutex;

	float volume_estimate;

	void calculate_audio_rms();

protected:
friend Source;

	void boop_thread();
	void remove_source(Source * source);
	void insert_source(Source * source);

	audio_tx_state_t state;

	// This buffer represents the 2x 20ms samples at 48kHz, 16 bit,
	// and is used to exchange data between the DMA buffer and the processing
	// task. 20ms is the length of a regular Opus frame decode.
	// The data here represents the interleaved left/right audio channels
	std::array<int16_t, 2 * XASAUDIO_TX_FRAME_SAMPLE_NO> audio_buffer;

	bool clipping;

	// These functions will always add exactly one frame (20ms) of the given data
	// buffer to the audio buffer. volume can be used to reduce the volume
	// of the given sample (independently of the global volume).
	// This function handles the left/right interleaved nature and avoids
	// saturation.
	void add_interleaved_frame(const int16_t *data, uint8_t volume = 255);
	void add_lr_frame(const int16_t *data, bool left = true, uint8_t volume = 255);

public:
	const i2s_port_t i2s_port;

	bool calculate_volume;
	uint8_t volume_mod;

	void audio_dma_fill_task();
	bool largestack_process();

	TX(i2s_port_t tx_port = I2S_NUM_0);
	TX(const TX&) = delete;

	void init(TaskHandle_t processing_task, const i2s_pin_config_t &pin_config);

	float get_volume_estimate();
	bool had_clipping();

	template<class T> Source * play(const T &sample, bool auto_delete = true);
};

}
}

#endif
