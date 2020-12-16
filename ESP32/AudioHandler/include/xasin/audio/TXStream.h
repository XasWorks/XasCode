/*
 * TXStream.h
 *
 *  Created on: 24 Nov 2020
 *      Author: xasin
 */

#ifndef ESP32_AUDIOHANDLER_TXSTREAM_H_
#define ESP32_AUDIOHANDLER_TXSTREAM_H_

#include "Source.h"
#include "AudioTX.h"

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#include "opus.h"
#include <array>

#define XASAUDIO_TX_STREAM_MAX_PACKETSIZE (1 + (CONFIG_XASAUDIO_TX_FRAMELENGTH * CONFIG_XASAUDIO_TX_STREAM_MAX_BITRATE) / (8 * 1000))

namespace Xasin {
namespace Audio {

struct stream_opus_packet_t {
	uint16_t length;
	std::array<uint8_t, XASAUDIO_TX_STREAM_MAX_PACKETSIZE> data;
};

class TXStream : public Source {
private:
	OpusDecoder * decoder;

	SemaphoreHandle_t packet_semaphore;

	int packet_tail;
	int packet_head;
	int packet_count;
	std::array<stream_opus_packet_t, 1 + (CONFIG_XASAUDIO_TX_STREAM_BUFFER_LENGTH / CONFIG_XASAUDIO_TX_FRAMELENGTH)> packet_buf;

	int packet_low_watermark;
	int packet_period_watermark;

	uint32_t decode_buffer_pos;
	std::array<int16_t, XASAUDIO_TX_FRAME_SAMPLE_NO> decode_buffer;
	uint32_t decode_playback_speed;

	bool decode_next_packet();

	bool process_frame();

public:
	TXStream(TX &handler);
	~TXStream();

	void feed_packet(const uint8_t * packet_ptr, uint16_t packet_size);
	void feed_packets(const uint8_t * data_ptr, uint16_t packet_size, uint8_t packet_count);

	bool is_finished();
	bool has_audio();
};

} /* namespace Audio */
} /* namespace Xasin */

#endif /* ESP32_AUDIOHANDLER_TXSTREAM_H_ */
