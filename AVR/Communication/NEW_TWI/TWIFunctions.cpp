
#include "TWIFunctions.h"

namespace TWI {
	nextTWIAction nextAction = TRANSFER;

	uint8_t targetAddr 	= 0;
	uint8_t targetReg		= 0;
	uint8_t dataLength 	= 0;

	uint8_t *dataPacket 	= 0;

	void fireTWINT(nextTWIAction nAct) {
		switch(nAct) {
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
			TWCR = TWCR_ON;
		break;
		}
	}
	void fireTWINT() {
		fireTWINT(nextAction);
	}

	Status readSR() {
		return TWSR & 0b11111000;
	}

	void updateTWI() {
		// Switch to the main couple of TWI-Codes that can occour
		switch(readSR()) {
			// Send the SLA-R/W addr after any (re)start
			// This assumes a job has configured itself & the address!
			case REPSTART:
			case START:
				TWDR = targetAddr;
				fireTWINT(TRANSFER);
			break;

			// Send the target register address (Register Concept)
			case MT_SLA_ACK:
				TWDR = targetReg;
				fireTWINT(TRANSFER);
			break;

			// Continually send data bytes from dataPacket while the Slave returns ACK
			case MT_DATA_ACK:
				if(dataLength == 0)
					// If there is no data left to be sent, check what the job wants to do next.
					// This can also include starting a next job via REPSTART!!
					masterWrapup();
				else {
					TWDR = *(dataPacket++);
					dataLength--;
					fireTWINT(TRANSFER);
				}
			break;

			// Continually read in data bytes from the SLA-R
			case MR_SLA_ACK:
			case MR_DATA_ACK:
				if(dataLength == 0)
					// Once all data has been filled, see what the job wants to do next!
					// Otherwise, check if there is a next job that gets a REPSTART
					masterWrapup();
				else {
					*(dataPacket++) = TWDR;
					dataLength--;
					fireTWINT(TRANSFER);
				}
			break;

			// SLA+R has been received, prepare for transmission!
			case SR_SLA_ACK:
				dataLength == 255;
				fireTWINT(TRANSFER);
			break;

			// Handle incoming SLA+R bytes!
			case SR_DATA_ACK:
				// If this is the first data byte (no Job configured), set targetAddr to the first data byte
				// Then let the Jobs prepare a data pointer to be written in to (Register Concept).
				if(dataLength == 255) {
					targetReg = TWDR;
					slaGetCallback();
					// Check if there even was any job that configued itself, otherwise NACK.
					fireTWINT(dataLength == 255 ? NACK : TRANSFER);
				}

				// Should the slave have received all data (thusly dataLength = 0), send a NACK
				// - no furhter data can be received
				else if(dataLength == 0)
					fireTWINT(NACK);
				// Otherwise, happily feed in the incoming data bytes into the dataPacket pointer
				else {
					*(dataPacket++) = TWDR;
					dataLength--;
					fireTWINT(TRANSFER);
				}
			break;

			// Once a SLA+R STOP has been received, let the Job do stuff with the received data!
			case SR_DATA_STOP:
				slaRWrapup();
				fireTWINT(TRANSFER);
			break;

			// As SLA+W, it is assumed a SLA+R was issued beforehand,
			// configuring the data pointer from which to read (Register Concept)
			case ST_SLA_ACK:
			case ST_DATA_ACK:
				// If all data has been sent but MASTER requests more, answer with a NACK
				if(dataLength == 0) fireTWINT(NACK);
				// Otherwise happily send data bytes from dataPacket
				else {
					TWDR = *(dataPacket++);
					--dataLength;
					fireTWINT(TRANSFER);
				}
			break;

			// When all data is sent, no furhter action is required.
			// This should however NOT issue the default operation, so it is captured here.
			case ST_DATA_STOP: break;

			default:
				handleError();
			break;
		}
	}


	void masterWrapup() {
		fireTWINT(STOP);
	}
	void slaPrepare() {
	}
	void slaRWrapup() {
	}

	void handleError() {
	}
}
