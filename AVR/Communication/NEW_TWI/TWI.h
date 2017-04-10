

#ifndef TWI_FUNCTIONS_H
#define TWI_FUNCTIONS_H

#include <avr/io.h>
#include <avr/interrupt.h>

#include "Job.h"

#define TWCR_ON (1<< TWEA | 1<< TWEN | 1<< TWIE)

namespace TWI {
	enum nextTWIAction : uint8_t {
		TRANSFER,
		START,
		STOP,
		NACK,
	};

	enum TWIModeEnum : uint8_t {
		MODE_IDLE,			// Be idle (after a STOP)
		MODE_STARTING,		// A job is about to start!
		MODE_MT,			// "Addr" was a SLA+W, go into MT mode.
		MODE_MR_REG,		// "Addr" was a SLA+R, but we still need to write the register! (via MT)
		MODE_MR_RECEIVE,	// The Register has been sent, receive data after sending a SLA+R
	};
	
	enum Status : uint8_t {
		IDLE 		= 0b11111000,
		ERROR		= 0b00000000,
		BUSSTART 	= 0b00001000,
		REPSTART	= 0b00010000,

		MT_SLA_ACK		= 0b00011000,
		MT_SLA_NACK		= 0b00100000,
		MT_DATA_ACK		= 0b00101000,
		MT_DATA_NACK	= 0b00110000,

		MR_SLA_ACK		= 0b01000000,
		MR_SLA_NACK		= 0b01001000,
		MR_DATA_ACK		= 0b01010000,
		MR_DATA_NACK	= 0b01011000,

		ST_SLA_ACK		= 0b10101000,
		ST_DATA_ACK		= 0b10111000,
		ST_DATA_NACK	= 0b11000000,
		ST_DATA_STOP	= 0b11001000,

		SR_SLA_ACK		= 0b01100000,
		SR_DATA_ACK		= 0b10000000,
		SR_DATA_NACK	= 0b10001000,
		SR_DATA_STOP	= 0b10100000,

		SR_GC_ACK		= 0b01110000,
		SR_GC_DATA_ACK	= 0b10010000,
		SR_GC_DATA_NACK	= 0b10011000,
	};

	extern volatile uint8_t targetAddr;
	extern volatile uint8_t targetReg;

	extern volatile uint8_t dataLength;
	extern uint8_t * volatile dataPacket;

	Status readSR();

	void updateTWI();
	void init();
	void setAddr(uint8_t address);

	bool isActive();

	void checkMasterJobs();

	void sendPacketTo(uint8_t addr, uint8_t reg, void *dPacket, uint8_t length);
}

#endif
