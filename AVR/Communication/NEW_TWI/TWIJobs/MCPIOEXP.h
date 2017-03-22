/*
 * MCPIOEXP.h
 *
 *  Created on: 17.03.2017
 *      Author: xasin
 */

#ifndef AVR_COMMUNICATION_NEW_TWI_TWIJOBS_MCPIOEXP_H_
#define AVR_COMMUNICATION_NEW_TWI_TWIJOBS_MCPIOEXP_H_

#include "../Job.h"
#include "../TWI.h"

namespace TWI {

enum MCP_Next : uint8_t {
	PORTS 	= 0x14,
	DDRS	= 0x00,
	PINS	= 0x12,
	NONE	= 0xff,
	GPPU	= 0x0C,
};

class MCP_IOEXP: public Job {
private:
	const uint8_t address;

	uint16_t cPORTS = 0xffff;
	uint16_t cDDRS	= 0;

	uint8_t cData = 0b01;

	void loadNextAction();
	MCP_Next nextAction();

public:
	uint16_t PORTS 	= 0;
	uint16_t DDRS	= 0xffff;
	volatile uint16_t PINS	= 0;

	MCP_IOEXP(uint8_t addr);

	bool masterPrepare();
	bool masterEnd();

	void update();
};

} /* namespace TWI */

#endif /* AVR_COMMUNICATION_NEW_TWI_TWIJOBS_MCPIOEXP_H_ */
