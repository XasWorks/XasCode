/*
 * AudioHandler.h
 *
 *  Created on: 20 Jan 2019
 *      Author: xasin
 */

#ifndef XASLIBS_AUDIOHANDLER_AUDIOHANDLER_H_
#define XASLIBS_AUDIOHANDLER_AUDIOHANDLER_H_

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/i2s.h"

#include <vector>

namespace Xasin {
namespace Peripheral {

class AudioHandler;

class AudioCassette {
protected:
	friend AudioHandler;

	uint8_t const *readStart;
	uint8_t const *readHead;
	uint8_t const *readEnd;

	int16_t get_chunk();
	bool is_done();

public:
	uint8_t volume;

	AudioCassette(const uint8_t *start, size_t length, uint8_t volume = 50);
	AudioCassette(const AudioCassette &top);

	AudioCassette();
};

typedef std::vector<AudioCassette> CassetteCollection;

class AudioHandler {
private:
	TaskHandle_t audioTask;

	std::vector<AudioCassette> currentCassettes;

	int samplerate;
public:
	const i2s_port_t i2s_port;

	void _audio_task();

	AudioHandler(int samplerate = 44100, i2s_port_t i2s_port = I2S_NUM_0);

	void start_thread(const i2s_pin_config_t &pinCFG);

	void insert_cassette(const AudioCassette &cas);
	void insert_cassette(const CassetteCollection &cassettes);
};

} /* namespace Peripheral */
} /* namespace Xasin */

#endif /* XASLIBS_AUDIOHANDLER_AUDIOHANDLER_H_ */
