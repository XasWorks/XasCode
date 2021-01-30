/*
 * RX.h
 *
 *  Created on: 12 Dec 2020
 *      Author: xasin
 */

#ifndef ESP32_AUDIOHANDLER_RX_H_
#define ESP32_AUDIOHANDLER_RX_H_

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <driver/i2s.h>

#include <array>

#define XASAUDIO_RX_FRAME_SAMPLE_NO ((CONFIG_XASAUDIO_RX_SAMPLERATE * CONFIG_XASAUDIO_RX_FRAMELENGTH)/1000)

namespace Xasin {
namespace Audio {

typedef std::array<int16_t, XASAUDIO_RX_FRAME_SAMPLE_NO> rx_buffer_t;

class RX {
private:
	int array_read_cnt;

	std::array<uint8_t, 4 * 2 * XASAUDIO_RX_FRAME_SAMPLE_NO> raw_dma_buffer;

	std::array<rx_buffer_t, 4> audio_buffer;
	uint8_t buffer_fill_pos;
	uint8_t buffer_read_pos;

	bool is_running;
	float volume_estimate;

	TaskHandle_t audio_task;
	TaskHandle_t processing_task_handle;

	int32_t current_dc_value;

	void calculate_audio_rms();

public:
	const i2s_port_t i2s_port;
	uint16_t gain;

	void audio_dma_read_task();

	RX(i2s_port_t rx_port = I2S_NUM_1);
	RX(const RX&) = delete;

	// virtual ~RX();
	// TODO implement a good deconstructor. We don't need it for now

	void init(TaskHandle_t processing_task, const i2s_pin_config_t &pin_cfg);

	float get_volume_estimate();

	bool has_new_audio();
	const rx_buffer_t &get_buffer();

	void start();
	void stop();
};

} /* namespace Audio */
} /* namespace Xasin */

#endif /* ESP32_AUDIOHANDLER_RX_H_ */