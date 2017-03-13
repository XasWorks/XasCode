
#include "TWIFunctions.h"

namespace TWI {
	nextTWIAction nextAction = TRANSFER;


	void fireTWINT() {
		switch(nextAction) {
		case TRANSFER:
			TWCR = TWCR_ON;
		break;
		case START:
			TWCR = (TWCR_ON | 1<< TWSTA);
		break;
		case STOP:
			TWCR = (TWCR_ON | 1<< TWSTO);
		break;

		case NACK:
			TWCR = (1<< TWINT | 1<< TWEN | 1<< TWIE);
		break;
		}
	}

	Status readSR() {
		return TWSR & 0b11111000;
	}

	void load(uint8_t dataPacket) {
		TWDR = dataPacket;
	}
}
