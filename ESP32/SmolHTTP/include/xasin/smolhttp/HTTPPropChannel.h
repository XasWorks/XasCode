/*
 * HTTPPropChannel.h
 *
 *  Created on: 4 Aug 2019
 *      Author: xasin
 */

#ifndef ESP32_SMOLHTTP_HTTPPROPCHANNEL_H_
#define ESP32_SMOLHTTP_HTTPPROPCHANNEL_H_

#include "xasin/smolhttp/Endpoint.h"
#include "xasin/propertypoint/BaseChannel.h"

namespace Xasin {
namespace HTTP {

class HTTPPropChannel: public PropP::BaseChannel {
private:
	Endpoint property_endpoint;

public:
	HTTPPropChannel(Server &server, std::string uri, PropP::PropertyHandler &handler);
	virtual ~HTTPPropChannel();
};

} /* namespace HTTP */
} /* namespace Xasin */

#endif /* ESP32_SMOLHTTP_HTTPPROPCHANNEL_H_ */
