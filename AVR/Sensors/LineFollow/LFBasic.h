/*
 * LFBasic.h
 *
 *  Created on: Jan 3, 2016
 *      Author: xasin
 */

#ifndef CODE_LINEFOLLOW_LFBASIC_H_
#define CODE_LINEFOLLOW_LFBASIC_H_

#include <avr/io.h>


//LF_OK 		//The line is under the sensors and tracked
//LF_AMBIG 	//The line is currently in an ambiguous state, no clear decision can be made
//LF_LOST 	//No sensors have the line and it could be lost
//LF_INTSEC	//A intersection has been found!

#define LF_RIGHT 	127
#define LF_LEFT 	-127

namespace LF {

	enum Status : uint8_t {OK, AMBIGUOUS, LOST, INTSEC};

	class Basic {
	public:
		volatile Status	lineStatus = OK;
		volatile int8_t lineOffset = 0;

		Basic();
	};
}

#endif /* CODE_LINEFOLLOW_LFBASIC_H_ */
