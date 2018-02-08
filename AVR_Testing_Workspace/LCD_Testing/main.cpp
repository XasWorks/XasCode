
#include <avr/io.h>
#include <util/delay.h>

#include "AVR/LCD/LCD.h"
#include "AVR/TIMER/Timer1.h"

LCD screen(&PORTA);

ISR(TIMER1_COMPA_vect) {
	screen.update();
}

int main() {

	Timer1::enable_CTC(250);

	screen.writeString("Hello :D", 0);

	uint8_t i=0;
	while(1) {
		_delay_ms(50);
		screen.writeNum(i++, 16, 3);
	}

	return 1;
}
