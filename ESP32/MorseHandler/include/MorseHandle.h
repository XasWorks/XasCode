/*
 * MorseHandle.h
 *
 *  Created on: 21 Oct 2018
 *      Author: xasin
 */

#ifndef COMPONENTS_MORSEHANDLER_MORSEHANDLE_H_
#define COMPONENTS_MORSEHANDLER_MORSEHANDLE_H_

#include <string>
#include <functional>

#include "esp_timer.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

namespace Peripheral {

class MorseHandle {
private:
	TaskHandle_t decodeHandle;

	uint16_t currentCharacter;

	void commit_char(char c);
	void commit_word();

	std::string strBuffer;

public:
	const uint16_t dotTime;
	std::function<void (std::string &word)> word_callback;

	static char dots_to_char(uint16_t dots);

	void decode_thread();

	MorseHandle(uint16_t dotTime);

	TaskHandle_t getDecodeHandle();
};

} /* namespace Peripheral */

#endif /* COMPONENTS_MORSEHANDLER_MORSEHANDLE_H_ */
