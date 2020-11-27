/*
 * BitCasette.h
 *
 *  Created on: 20 Nov 2020
 *      Author: xasin
 */

#ifndef ESP32_AUDIOHANDLER_BITCASETTE_H_
#define ESP32_AUDIOHANDLER_BITCASETTE_H_

#include <xasin/audio/Source.h>
#include <stdint.h>

namespace Xasin {
namespace Audio {

struct bytecassette_data_t {
	const uint8_t *data_start;
	const uint8_t *data_end;
	uint32_t data_samplerate;
	uint8_t volume;
};

class ByteCassette: public Source {
private:
	const uint8_t *current_sample;
	const uint8_t * const sample_end;

	uint32_t sample_position_counter;
	const uint32_t per_sample_increase;

protected:
	bool process_frame();

public:
	const uint32_t data_samplerate;

	uint8_t volume;

	ByteCassette(TX &handler, const uint8_t *sample_ptr, const uint8_t *sample_end,
			uint32_t intended_samplerate);
	ByteCassette(TX &handler, const bytecassette_data_t &cassette);

	static void play(TX &handler, const bytecassette_data_t &cassette);

	~ByteCassette();

	bool is_finished();
};

} /* namespace Audio */
} /* namespace Xasin */

#endif /* ESP32_AUDIOHANDLER_BITCASETTE_H_ */
