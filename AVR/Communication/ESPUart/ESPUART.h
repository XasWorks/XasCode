/*
 * ESPUART.h
 *
 *  Created on: 21.07.2017
 *      Author: xasin
 */

#ifndef LOCALCODE_ESPCOMS_ESPUART_H_
#define LOCALCODE_ESPCOMS_ESPUART_H_

#include <avr/io.h>
#include <avr/interrupt.h>

#include "Endpoint.h"
#include "Source.h"

namespace ESPComs {

#ifndef ESP_BAUDRATE
#define ESP_BAUDRATE 31250
#endif
#ifndef ESP_START_CHAR
#define ESP_START_CHAR '!'
#endif

#if defined(__AVR_ATmega32__)
#define UBRR0 	UBRRL
#define UCSR0B	UCSRB
#define UCSR0C	UCSRC
#define UDR0	UDR

#define	UCSZ00	UCSZ0
#define UCSZ01	UCSZ1
#define UDRIE0	UDRIE

#define RXEN0	RXEN
#define TXEN0	TXEN
#define RXCIE0	RXCIE
#define	TXCIE0	TXCIE

#define USART_RX_vect USART_RXC_vect
#endif

enum RXStates {
	WAIT_FOR_START_RX,
	RX_COMMAND,
	RX_DATA
};

enum TXStates {
	WAIT_FOR_START_TX,
	TX_IDLE,
	TX_SENDING,
};

void increaseTimeout(uint16_t maxFrames);

void onReset(void (* const resetCallback)());

bool tryToStart(Source * txSource);
void init();

} /* namespace ESPComs */

#endif /* LOCALCODE_ESPCOMS_ESPUART_H_ */
