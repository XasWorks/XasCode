/*
 * Job.cpp
 *
 *  Created on: 16.03.2017
 *      Author: xasin
 */

#include "Job.h"

namespace TWI {

Job * Job::headJob = 0;

Job::Job() {
	this->nextJob = headJob;
	headJob = this;
}

Job * Job::getHeadJob() {
	return headJob;
}
Job * Job::getNextJob() {
	return this->nextJob;
}

} /* namespace TWI */
