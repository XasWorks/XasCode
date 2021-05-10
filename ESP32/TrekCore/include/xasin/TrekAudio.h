/*
 * TrekAudio.h
 *
 *  Created on: 27 Feb 2019
 *      Author: xasin
 */

#ifndef XASLIBS_TREKCORE_INCLUDE_XASIN_TREKAUDIO_H_
#define XASLIBS_TREKCORE_INCLUDE_XASIN_TREKAUDIO_H_

#include <xasin/audio.h>

#define TREKCORE_AUDIO_ENABLED

namespace Xasin {
namespace Trek {

enum signal_type_t {
	KEYPRESS,
	INPUT_OK,
	INPUT_REQ,
	INPUT_BAD,
	PROG_BUSY,
	PROG_DONE,
	PROG_FAILED,
	ERROR_MAJOR,
};

void init(Audio::TX &audio);

void play(signal_type_t signal);

}
}


#endif /* XASLIBS_TREKCORE_INCLUDE_XASIN_TREKAUDIO_H_ */
