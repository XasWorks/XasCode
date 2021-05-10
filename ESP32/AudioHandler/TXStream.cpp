/*
 * TXStream.cpp
 *
 *  Created on: 24 Nov 2020
 *      Author: xasin
 */

#include "xasin/audio/TXStream.h"
#include <cstring>

#define LOG_LOCAL_LEVEL ESP_LOG_INFO
#include "esp_log.h"

namespace Xasin {
namespace Audio {

TXStream::TXStream(TX &handler) : Source(handler),
		packet_tail(0), packet_head(0), packet_count(0),
		packet_buf(),
		packet_low_watermark(0), packet_period_watermark(0),
		decode_buffer_pos(UINT32_MAX), decode_buffer(), decode_playback_speed(1<<16) {

	decoder = opus_decoder_create(CONFIG_XASAUDIO_TX_SAMPLERATE, 1, nullptr);
	if(decoder <= 0)
		ESP_LOGE("audio_tx", "Decoder could not be created!");

	packet_semaphore = xSemaphoreCreateMutex();
}

TXStream::~TXStream() {
	packet_count = 0;

	opus_decoder_destroy(decoder);

	xSemaphoreTake(packet_semaphore, portMAX_DELAY);
	xSemaphoreGive(packet_semaphore);

	vSemaphoreDelete(packet_semaphore);
}

bool TXStream::decode_next_packet() {
	if(packet_count == 0)
		return false;

	xSemaphoreTake(packet_semaphore, portMAX_DELAY);

	stream_opus_packet_t packet_copy = packet_buf[packet_tail];
	packet_tail = (packet_tail + 1) % packet_buf.size();
	packet_count--;

	if(packet_count < packet_low_watermark)
		packet_low_watermark = packet_count;
	if(packet_count < packet_period_watermark)
		packet_period_watermark = packet_count;

	if((packet_tail >> 2) == 0) {
		packet_low_watermark = packet_period_watermark;
		packet_period_watermark = packet_buf.size();
	}

	int low_watermark_fill_percent = (packet_low_watermark << 8) / packet_buf.size();
	ESP_LOGV("TXStream", "Audio low watermark fill percent %d", low_watermark_fill_percent);

	decode_playback_speed = (1<<16) + (low_watermark_fill_percent - 51) * 12.85;

	xSemaphoreGive(packet_semaphore);

	auto retcode = opus_decode(decoder, packet_copy.data.data(), packet_copy.length,
			decode_buffer.data(), decode_buffer.size(), 0);

	if(retcode < 0)
		return false;

	decode_buffer_pos &= 0xFFFF;

	return true;
}

bool TXStream::process_frame() {
	if((packet_count == 0) && (decode_buffer_pos >= XASAUDIO_TX_FRAME_SAMPLE_NO))
		return false;

	std::array<int16_t, XASAUDIO_TX_FRAME_SAMPLE_NO> frame_buffer;

	for(int i = 0; i<frame_buffer.size(); i++) {
		if((decode_buffer_pos >> 16) >= decode_buffer.size()) {
			if(!decode_next_packet())
				break;
		}

		frame_buffer[i] = decode_buffer[decode_buffer_pos >> 16];
		decode_buffer_pos += decode_playback_speed;
	}

	add_lr_frame(frame_buffer.data());

	return true;
}

void TXStream::feed_packet(const uint8_t * packet_ptr, uint16_t packet_size) {
	if(packet_size >= XASAUDIO_TX_STREAM_MAX_PACKETSIZE)
		return;

	xSemaphoreTake(packet_semaphore, portMAX_DELAY);

	memcpy(packet_buf[packet_head].data.data(), packet_ptr, packet_size);
	packet_buf[packet_head].length = packet_size;

	packet_head = (1 + packet_head) % packet_buf.size();

	if(packet_count == packet_buf.size())
		packet_tail = (1 + packet_tail) % packet_buf.size();
	else
		packet_count++;

	xSemaphoreGive(packet_semaphore);

	if(packet_count > packet_buf.size() / 2)
		boop_playback();
}

void TXStream::feed_packets(const uint8_t * data_ptr, uint16_t packet_size, uint8_t packet_no) {
	ESP_LOGV("TXStream", "Attempting to feed %d packets with size %d", packet_no, packet_size);

	if(packet_size >= XASAUDIO_TX_STREAM_MAX_PACKETSIZE)
		return;
	if(packet_no == 0)
		return;

	xSemaphoreTake(packet_semaphore, portMAX_DELAY);

	if(packet_no > packet_buf.size()) {
		data_ptr += packet_size * (packet_no - packet_buf.size());
		packet_no = packet_buf.size();
	}

	packet_count += packet_no;
	if(packet_count > packet_buf.size()) {
		packet_tail = (packet_tail + packet_count - packet_buf.size()) % packet_buf.size();
		packet_count = packet_buf.size();
	}

	for(; packet_no != 0; packet_no--) {
		memcpy(packet_buf[packet_head].data.data(), data_ptr, packet_size);
		packet_buf[packet_head].length = packet_size;

		packet_head = (1 + packet_head) % packet_buf.size();

		data_ptr += packet_size;
	}

	ESP_LOGV("TXStream", "Fed %d packets, total count now %d", packet_no, packet_count);

	xSemaphoreGive(packet_semaphore);

	if(packet_count > packet_buf.size() / 2)
		boop_playback();
}

bool TXStream::is_finished() {
	return false;
}
bool TXStream::has_audio() {
	return packet_count > 0;
}

} /* namespace Audio */
} /* namespace Xasin */
