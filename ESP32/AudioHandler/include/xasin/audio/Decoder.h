/*
 * Decoder.h
 *
 *  Created on: 8 Nov 2020
 *      Author: xasin
 */

#ifndef ESP32_AUDIOHANDLER_DECODER_H_
#define ESP32_AUDIOHANDLER_DECODER_H_

namespace Xasin {
namespace Audio {
namespace OPUS {

class Decoder {
public:
	Decoder();
	virtual ~Decoder();
};

} /* namespace OPUS */
} /* namespace Audio */
} /* namespace Xasin */

#endif /* ESP32_AUDIOHANDLER_DECODER_H_ */
