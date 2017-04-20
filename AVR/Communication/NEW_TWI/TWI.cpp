
#include "TWI.h"
#include <util/delay.h>

namespace TWI {
	Job * callbackJob = 0;

	volatile uint8_t targetAddr 	= 0;
	volatile uint8_t targetReg	= 0;
	volatile uint8_t dataLength 	= 0;

	uint8_t * volatile dataPacket = 0;

	volatile TWIModeEnum currentMode = MODE_IDLE;

	void fireTWINT(nextTWIAction nAct) {
		switch(nAct) {
		case TRANSFER:
			TWCR = (1<< TWINT | TWCR_ON);
		break;

		case START:
			TWCR = (TWCR_ON | 1<< TWSTA | 1<< TWINT);
		break;

		case STOP:
			currentMode = MODE_IDLE;
			TWCR = (1<< TWINT | TWCR_ON | 1<< TWSTO);
		break;

		case NACK:
			TWCR = (1<< TWINT | 1<< TWEN | 1<< TWIE);
		break;
		}
	}

	Status readSR() {
		return (Status)(TWSR & 0b11111000);
	}

	bool startNewJob() {
		Job * jobNode = Job::getHeadJob();

		while(jobNode != 0) {
			if(jobNode->masterPrepare()) {

				callbackJob = jobNode;
				currentMode = MODE_STARTING;
				return true;
			}
			jobNode = jobNode->getNextJob();
		}

		return false;
	}

	void masterWrapup() {
		// Check if there is a callback function. Can happen there's not!
		if(callbackJob != 0)
			if(callbackJob->masterEnd()) {
				fireTWINT(START);
				return;
			}

		callbackJob = 0;
		// If the callback job has NOT selected to continue (via REPSTART), see if there is any other job willing to start!

		if(startNewJob())
			fireTWINT(START);
		else
			fireTWINT(STOP);
	}

	void slaGetJob() {
		callbackJob = Job::getHeadJob();
		while(callbackJob != 0) {
			if(callbackJob->slavePrepare()) {
				fireTWINT(TRANSFER);
				return;
			}
			callbackJob = callbackJob->getNextJob();
		}

		fireTWINT(NACK);
	}
	void slaWrapup() {
	}

	void handleError() {
		fireTWINT(STOP);

		if(callbackJob != 0)
			callbackJob->error();
	}

	void updateTWI() {
		// Switch to the main couple of TWI-Codes that can occur
		switch(readSR()) {
			// Send the SLA-R/W addr after any (re)start
			// This assumes a job has configured itself & the address!
			case REPSTART:
			case BUSSTART:
				if((targetAddr & 0b1) == 0) {
					TWDR = targetAddr;
					currentMode = MODE_MT;
				}
				else {
					if(currentMode == MODE_MR_RECEIVE)
						TWDR = targetAddr;
					else {
						TWDR = targetAddr & 0b11111110;
						currentMode = MODE_MR_REG;
					}
				}

				fireTWINT(TRANSFER);
			break;

			// Send the target register address (Register Concept)
			case MT_SLA_ACK:
				TWDR = targetReg;
				fireTWINT(TRANSFER);
			break;

			// Continually send data bytes from dataPacket while the Slave returns ACK
			case MT_DATA_ACK:
				if(currentMode == MODE_MR_REG) {
					currentMode = MODE_MR_RECEIVE;
					fireTWINT(START);
				}
				else if(dataLength == 0)
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
			case MR_DATA_ACK:
				*(dataPacket++) = TWDR;
				dataLength--;
			case MR_SLA_ACK:
				if(dataLength == 1)
					fireTWINT(NACK);
				else
					fireTWINT(TRANSFER);
			break;

			case MR_DATA_NACK:
				*(dataPacket++) = TWDR;
				dataLength--;

				masterWrapup();
			break;

			// SLA+R has been received, prepare for transmission!
			case SR_SLA_ACK:
				dataLength = 255;
				fireTWINT(TRANSFER);
			break;

			// Handle incoming SLA+R bytes!
			case SR_DATA_ACK:
				// If this is the first data byte (no Job configured), set targetAddr to the first data byte
				// Then let the Jobs prepare a data pointer to be written in to (Register Concept).
				if(dataLength == 255) {
					targetReg = TWDR;
					slaGetJob();
				}
				else {
					// Should the slave have received all data (thusly dataLength = 0), ask the job what to do
					if(dataLength == 0)
						slaWrapup();

					// After the job was able to re-set a data pointer, check if there ACTUALLY is new data!
					// Otherwise, happily feed in the incoming data bytes into the dataPacket pointer
					if(dataLength != 0){
						*(dataPacket++) = TWDR;
						dataLength--;
					}
				}

				fireTWINT(TRANSFER);
			break;

			// Once a SLA+R STOP has been received, let the Job do stuff with the received data!
			case SR_DATA_STOP:
				slaWrapup();
				fireTWINT(TRANSFER);
			break;

			// As SLA+W, it is assumed a SLA+R was issued beforehand,
			// configuring the data pointer from which to read (Register Concept)
			case ST_SLA_ACK:
			case ST_DATA_ACK:
				// If all data has been sent but MASTER requests more, check what the job wants to do
				if(dataLength == 0) {
					slaWrapup();
				}

				if(dataLength == 0)
					TWDR = 0xff;
				// Otherwise happily send data bytes from dataPacket
				else {
					TWDR = *(dataPacket++);
					--dataLength;
				}
				fireTWINT(TRANSFER);
			break;

			case ST_DATA_NACK:
				TWDR = 0xff;
				fireTWINT(TRANSFER);
			break;

			// When all data is sent, no furhter action is required.
			// This should however NOT issue the default operation, so it is captured here.
			case ST_DATA_STOP:
				fireTWINT(TRANSFER);
			break;

			default:
				handleError();
			break;
		}
	}

	void init() {
		// Activate TWI and Interrupt
		TWCR = (1<< TWEN | 1<< TWIE | 1<< TWEA);


		TWBR = 10;

		// Enable Pullups
#if defined (__AVR_ATmega328P__)
		PORTC |= (0b11 << PC4);
#elif defined (__AVR_ATmega32__) || defined (__AVR_ATmega644P__)
		PORTC |= (0b11 << PC0);
#else
#warning No fitting Pull-Ups defined for this MCU yet!
#endif

		sei();
	}

	void setAddr(uint8_t address) {
		TWAR = address << 1;
	}

	bool isActive() {
		return currentMode != MODE_IDLE;
	}

	void checkMasterJobs() {
		if(isActive())
			return;

		if(startNewJob())
			fireTWINT(START);
	}

}
