
#ifndef TIMER_DEFS_H
#define TIMER_DEFS_H

#if defined(__AVR_ATmega32__)

	#define TIMER1_ENABLED
	#define TINTMASK TIMSK

#elif defined(__AVR_ATmega328P__) || (__AVR_ATmega644P__)
	#define TIMER0_ENABLED

	#define TIMER1_ENABLED
	#define TINTMASK TIMSK1

	#define TIMER2_ENABLED
#endif


#endif
