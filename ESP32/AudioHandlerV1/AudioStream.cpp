/*
 * AudioStream.cpp
 *
 *  Created on: 5 Sep 2020
 *      Author: xasin
 */

#include "AudioStream.h"
#include <algorithm>

#include <cstring>
#include <cmath>

#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG
#include "esp_log.h"

namespace Xasin {
namespace Peripheral {

AudioStream::AudioStream(AudioHandler &handler, MQTT::Handler &mqtt) :
	AudioSample(),
	handler(handler), mqtt(mqtt),
	samp_buffer(),
	sample_head(0), sample_tail(0), sample_count(0),
	speed_adjust(0),
	last_packet_id(0) {
}

AudioStream::~AudioStream() {
}

bool AudioStream::is_done() {
	//return sample_count <= 0;
	return false;
}

#define PER_SAMP_INCREASE ((0x1000 * uint32_t(24000))/44100)

int16_t AudioStream::get_chunk() {
	if(sample_count <= 0)
		return 0;

	const uint32_t sample_pos = sample_tail >> 12;

	const int16_t rData = samp_buffer[sample_pos];
	const int16_t lData = samp_buffer[(sample_pos+1) & (samp_buffer.size()-1)];

	const int32_t split = (sample_pos & 0xFF0) >> 4;

	const int32_t out_data = (255-split) * rData + split * lData;

	sample_tail   = (sample_tail + PER_SAMP_INCREASE + speed_adjust) & (((samp_buffer.size())<<12) - 1);
	sample_count -= PER_SAMP_INCREASE + speed_adjust;

	if(sample_count < 0)
		ESP_LOGD("AudioStream", "Audio stream ran dry!");

	return out_data >> 8;
}

int log_prescaler = 0;
TickType_t last_rec = 0;

void AudioStream::add_samples(const int16_t *sample_ptr, size_t count) {
	if(count < 0)
		return;

	handler.get_audio_lock();

	if(count == 0) {
		last_packet_id = 0;
		sample_count = 0;
		sample_head  = 0;
		sample_tail  = 0;
	}

	if(count > samp_buffer.size()) {
		sample_ptr += (count - samp_buffer.size());
		count = samp_buffer.size();
	}

	while(count > 0) {
		size_t edge_fill_cnt = std::min(samp_buffer.size() - sample_head, count);
		memcpy(samp_buffer.data() + sample_head, sample_ptr, edge_fill_cnt * 2);

		sample_ptr += edge_fill_cnt;
		count -= edge_fill_cnt;

		sample_count += edge_fill_cnt << 12;
		sample_head = (sample_head + edge_fill_cnt) & (samp_buffer.size() - 1);
	}

	int32_t overflow_cnt = sample_count - ((samp_buffer.size() << 12) - 1);
	if(overflow_cnt > 0) {
		sample_tail = (sample_tail + overflow_cnt) & (((samp_buffer.size())<<12) - 1);
		sample_count = (samp_buffer.size() << 12) - 1;

		ESP_LOGD("AudioStream", "Audio stream overflowed!");
	}

	const int8_t percent_fill = ((sample_count >> 12) * 100) / (samp_buffer.size());
	speed_adjust = (percent_fill - 70) * 1;

	handler.release_audio_lock();

	TickType_t this_rec = xTaskGetTickCount();
	ESP_LOGD("AudioStream", "Got packet after %d ms (%d%%)", this_rec - last_rec, percent_fill);
	last_rec = this_rec;

	if(sample_count > (samp_buffer.size() << 11))
		handler.notify_task();
}

} /* namespace Peripheral */
} /* namespace Xasin */
