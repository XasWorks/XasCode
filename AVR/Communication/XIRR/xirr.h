/*
 * xirr.h
 *
 *  Created on: 31 May 2019
 *      Author: xasin
 */

#ifndef XASLIBS_COMMUNICATION_XIRR_XIRR_H_
#define XASLIBS_COMMUNICATION_XIRR_XIRR_H_

#include <avr/io.h>

namespace Communication {
namespace XIRR {

enum Channels : uint8_t {
	IR_BACON = 128,
	LZRTAG_BEACON = 129,
};

namespace RX {

extern uint8_t buffer[32];
extern void (*process_data)(const uint8_t *data, uint8_t length, uint8_t key);

void update();
void pin_capture();

}

namespace TX {
void update();

void send_raw(const uint8_t *data, uint8_t length);

void send(const void *data, uint8_t length, uint8_t key);
}

void init();

bool is_idle();

}
}


#endif /* XASLIBS_COMMUNICATION_XIRR_XIRR_H_ */
