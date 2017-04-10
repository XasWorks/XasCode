/*
 * MCPIOEXP.cpp
 *
 *  Created on: 17.03.2017
 *      Author: xasin
 */

#include "MCPIOEXP.h"

namespace TWI {

uint8_t gpREG = 0xff;

MCP_IOEXP::MCP_IOEXP(uint8_t addr) : address(addr) {

}

MCP_Next MCP_IOEXP::nextAction() {
	if((this->cData & 0b10) == 0)
		return MCP_Next::GPPU;

	if(this->cDDRS != this->DDRS)
		return MCP_Next::DDRS;
	if(this->cPORTS != this->PORTS)
		return MCP_Next::PORTS;

	if((this->cData & 0b1) == 0)
		return MCP_Next::PINS;

	return MCP_Next::NONE;
}

void MCP_IOEXP::loadNextAction() {
	TWI::targetAddr	= this->address;
	TWI::targetReg 	= (uint8_t)this->nextAction();
	TWI::dataLength	= 2;

	switch(this->nextAction()) {
	case MCP_Next::PORTS:
		this->cPORTS = this->PORTS;
		TWI::dataPacket = (uint8_t *)&this->cPORTS;
	break;
	case MCP_Next::DDRS:
		this->cDDRS = this->DDRS;
		TWI::dataPacket = (uint8_t *)&this->cDDRS;
	break;
	case MCP_Next::PINS:
		TWI::targetAddr = this->address | 1;
		TWI::dataPacket = (uint8_t *)&this->PINS;
	break;

	case MCP_Next::GPPU:
		TWI::dataLength = 1;
		TWI::dataPacket = &gpREG;
	break;

	default: break;
	}
}

bool MCP_IOEXP::masterPrepare() {
	if(TWI::isActive())
		return false;

	if(this->nextAction() == MCP_Next::NONE)
		return false;

	this->loadNextAction();
	return true;
}

bool MCP_IOEXP::masterEnd() {
	if(TWI::targetReg == MCP_Next::PINS)
		cData |= 1;
	if(TWI::targetReg == MCP_Next::GPPU)
		cData |= 0b10;

	return false;
}

void MCP_IOEXP::update() {
	this->cData &= ~(1);
}

} /* namespace TWI */
