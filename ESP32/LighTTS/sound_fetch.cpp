

#include "soundMap.h"

#include <cmath>

#define LOG_LOCAL_LEVEL ESP_LOG_INFO
#include "esp_log.h"

namespace Xasin {
namespace LighTTS {

Peripheral::AudioHandler *audioHandler = nullptr;

const Peripheral::AudioCassette get_sample_for(std::string key) {
	if(lightts_available_samples.count(key) == 0)
		return Peripheral::AudioCassette();

	audio_samp_t audio = lightts_available_samples.at(key);

	return Peripheral::AudioCassette(audio.audioStart, audio.length, 100, LIGHTTS_PRESCALING_FACTOR);
}

void speak_with_pause(std::string key) {
	if(lightts_available_samples.count(key) == 0 || audioHandler == nullptr) {
		vTaskDelay(pdMS_TO_TICKS(50));
		return;
	}


	audio_samp_t audio = lightts_available_samples.at(key);

	size_t reqDelay = (LIGHTTS_PRESCALING_FACTOR*audio.length)/44.1;
	audioHandler->insert_cassette(Peripheral::AudioCassette(audio.audioStart, audio.length, 130, LIGHTTS_PRESCALING_FACTOR));

	vTaskDelay(pdMS_TO_TICKS(reqDelay + 100));
}

void speak_number(float number, bool point_digits = true) {
	ESP_LOGD("LIGHTTS", "Speaking number %f", number);

	if(number < 0) {
		speak_with_pause("minus");
		speak_number(-number);
		return;
	}

	if(number < 0.000001) {
		speak_with_pause("0");
	}

	if(number < 0.001) {
		speak_number(number * 1000000);
		speak_with_pause("micro");
		return;
	}
	else if(number < 1) {
		speak_number(number * 1000);
		speak_with_pause("milli");
		return;
	}

	if(number >= 1000000) {
		speak_number(fmod(number/1000000, 100), false);
		speak_with_pause("million");
		vTaskDelay(300);

		number = fmod(number, 1000000);
	}
	if(number >= 1000) {
		speak_number(fmod(number/1000, 100), false);
		speak_with_pause("thousand");
		vTaskDelay(300);

		number = fmod(number, 1000);
	}

	int hundreds = floor(number / 100);
	if(hundreds > 0) {
		char oString[] = {static_cast<char>(hundreds + '0'), 0};
		speak_with_pause(oString);
		speak_with_pause("hundred");
		vTaskDelay(140);
	}
	number = fmod(number, 100);

	int tens = floor(number / 10);
	if(tens > 0) {
		char oString[] = {static_cast<char>(tens + '0'), '0', 0};
		speak_with_pause(oString);
	}
	number = fmod(number, 10);

	int ones = floor(number);
	if(ones > 0) {
		char oString[] = {static_cast<char>(ones + '0'), 0};
		speak_with_pause(oString);
	}

	if(!point_digits)
		return;

	int digit_number = fmod(round(number*1000), 1000);
	if(digit_number == 0)
		return;

	vTaskDelay(100);
	speak_with_pause("point");
	vTaskDelay(300);

	for(uint8_t i=0; i<3; i++) {
		char oString[] = {static_cast<char>(digit_number/100 + '0'), 0};
		speak_with_pause(oString);

		digit_number = fmod(digit_number*10, 100);
		if(digit_number == 0)
			break;
	}
}

bool is_number(std::string word) {

	for(uint8_t i=0; i<word.length(); i++) {
		char c = word.at(i);

		if(c == '-' && i == 0)
			continue;
		if((c >= '0') && (c <= '9'))
			continue;
		if(c == '.')
			continue;

		return false;
	}

	return true;
}

void speak_string(std::string text) {
	int lastPos = 0;

	while(true) {
		auto argStr = text.substr(lastPos,
			text.find(" ", lastPos) - lastPos);

		TickType_t extra_pause = 0;
		auto argLen = argStr.length();
		if(argStr.at(argLen-1) == '.') {
			argStr = argStr.substr(0, argStr.length()-1);
			extra_pause = 500;
		}
		else if(argStr.at(argStr.length()-1) == ',') {
			argStr = argStr.substr(0, argStr.length()-1);
			extra_pause = 300;
		}

		ESP_LOGD("LIGHTTS", "Speaking word: %s", argStr.data());

		if(is_number(argStr))
			speak_number(atof(argStr.data()));
		else
			speak_with_pause(argStr);

		vTaskDelay(extra_pause);

		lastPos = text.find(" ", lastPos);
		if(lastPos > text.length())
			break;

		while(text.at(lastPos) == ' ')
			lastPos++;

		if(lastPos >= text.length())
			break;
	}
}

}
}
