/*
 * MorseHandle.cpp
 *
 *  Created on: 21 Oct 2018
 *      Author: xasin
 */

#define D_TO_CH(DOT, CHAR) case DOT: return CHAR; break

#include "xasin/MorseHandle.h"

namespace Peripheral {

char MorseHandle::dots_to_char(uint16_t dots) {
	switch(dots) {
	D_TO_CH(0b101, 'a');
	D_TO_CH(0b11000, 'b');
	D_TO_CH(0b11010, 'c');
	D_TO_CH(0b1100,  'd');
	D_TO_CH(0b10, 	 'e');
	D_TO_CH(0b10010, 'f');
	D_TO_CH(0b1110,  'g');
	D_TO_CH(0b10000, 'h');
	D_TO_CH(0b100, 	 'i');
	D_TO_CH(0b10111, 'j');
	D_TO_CH(0b1101,  'k');
	D_TO_CH(0b10100, 'l');
	D_TO_CH(0b111,   'm');
	D_TO_CH(0b110,   'n');
	D_TO_CH(0b1111,  'o');
	D_TO_CH(0b10110, 'p');
	D_TO_CH(0b11101, 'q');
	D_TO_CH(0b1010,  'r');
	D_TO_CH(0b1000,  's');
	D_TO_CH(0b11,    't');
	D_TO_CH(0b1001,  'u');
	D_TO_CH(0b10001, 'v');
	D_TO_CH(0b1011,  'w');
	D_TO_CH(0b11001, 'x');
	D_TO_CH(0b11011, 'y');
	D_TO_CH(0b11100, 'z');
	default:
		return '?';
	}
}

void MorseHandle::decode_thread() {
	uint32_t touched = false;

	uint64_t lastChangeTime = esp_timer_get_time()/1000;
	while(true) {
		touched = false;
		while(!touched)
			xTaskNotifyWait(0, 0, &touched, portMAX_DELAY);

		while(true) {
			lastChangeTime = esp_timer_get_time()/1000;
			auto ret = xTaskNotifyWait(0, 0, &touched, dotTime*7);
			if(ret == pdFAIL)
				touched = !touched;

			uint32_t touchTime = (esp_timer_get_time()/1000) - lastChangeTime;

			if(touched) {
				if(touchTime > 2.5*dotTime) {
					commit_char(dots_to_char(currentCharacter));
					if(touchTime > 6.5*dotTime)
						commit_word();
				}
			}
			else {
				if(touchTime < 1.2*dotTime)
					currentCharacter <<= 1;
				else if(touchTime < 6*dotTime)
					currentCharacter = (currentCharacter << 1) | 1;
				else {
					commit_char('!');
					ret = pdPASS;
					xTaskNotifyWait(0, 0, nullptr, portMAX_DELAY);
				}
			}

			if(ret == pdFAIL)
				break;
		}
	}
}

void MorseHandle::commit_char(char c) {
	currentCharacter = 1;
	if(c == '?')
		return;

	strBuffer.push_back(c);
}
void MorseHandle::commit_word() {
	printf("M: Word: %s\n", strBuffer.c_str());
	if(word_callback != nullptr)
		word_callback(strBuffer);

	strBuffer.clear();
}

MorseHandle::MorseHandle(uint16_t dotTime) : dotTime(dotTime), word_callback(nullptr) {
	xTaskCreate([](void *args) {
		((MorseHandle*)args)->decode_thread();
	}, "Morse Decode", 2048, this, 1, &decodeHandle);

	currentCharacter = 1;

	strBuffer.clear();
	strBuffer.reserve(10);
}

TaskHandle_t MorseHandle::getDecodeHandle() {
	return decodeHandle;
}

} /* namespace Peripheral */
