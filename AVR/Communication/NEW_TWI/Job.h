/*
 * Job.h
 *
 *  Created on: 16.03.2017
 *      Author: xasin
 */

#ifndef AVR_COMMUNICATION_NEW_TWI_JOB_H_
#define AVR_COMMUNICATION_NEW_TWI_JOB_H_

#include <avr/io.h>
#include "TWI.h"

namespace TWI {

class Job {
private:
	static Job * headJob;
	Job * nextJob;

public:
	Job();

	Job * getNextJob();
	static Job * getHeadJob();

	virtual bool masterPrepare();
	virtual bool slavePrepare();

	virtual bool masterEnd();
	virtual void slaveEnd();

	virtual void error();
};

} /* namespace TWI */

#endif /* AVR_COMMUNICATION_NEW_TWI_JOB_H_ */
