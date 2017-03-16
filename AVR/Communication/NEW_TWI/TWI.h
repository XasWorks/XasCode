

#ifndef TWI_FUNCTIONS_H
#define TWI_FUNCTIONS_H

#include <avr/io.h>
#include <avr/interrupt.h>

#define TWCR_ON (1<< TWINT | 1<< TWEA | 1<< TWEN | 1<< TWIE)

namespace TWI {
	enum nextTWIAction : uint8_t {
		TRANSFER,
		START,
		STOP,
		NACK,
	};

	extern nextTWIAction nextAction;

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

		SR_GC_ACK			= 0b01110000,
		SR_GC_DATA_ACK		= 0b10010000,
		SR_GC_DATA_NACK	= 0b10011000,
	};

	extern uint8_t targetAddr;

	extern uint8_t dataLength;
	extern uint8_t *dataPacket;

	Status readSR();

	void updateTWI();
	void init();

	void sendPacketTo(uint8_t addr, uint8_t reg, uint8_t *dataPacket, uint8_t length);
}

#endif
