/*
 * AudioStream.h
 *
 *  Created on: 5 Sep 2020
 *      Author: xasin
 */

#ifndef ESP32_AUDIOHANDLER_AUDIOSTREAM_H_
#define ESP32_AUDIOHANDLER_AUDIOSTREAM_H_

#include <AudioHandler.h>
#include <xasin/mqtt/Handler.h>

#include <array>

namespace Xasin {
namespace Peripheral {

class AudioStream: public AudioSample {
private:
	AudioHandler &handler;
	MQTT::Handler &mqtt;

	std::array<int16_t, 16384> samp_buffer;

	// Fixcomma number, represents buffer pos * 1024
	uint32_t sample_head;
	uint32_t sample_tail;
	int32_t  sample_count;

	int16_t  speed_adjust;

	int32_t last_packet_id;

protected:
	bool is_done();
	int16_t get_chunk();

public:
	AudioStream(AudioHandler &handler, MQTT::Handler &mqtt);
	virtual ~AudioStream();

	void add_samples(const int16_t *samp_ptr, size_t count);
};

} /* namespace Peripheral */
} /* namespace Xasin */

#endif /* ESP32_AUDIOHANDLER_AUDIOSTREAM_H_ */
