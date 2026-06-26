/*
 * DS1302.c
 *
 * Created: 2026-06-26 오후 2:37:54
 * Author : kccistc
 */ 
#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>

#define STABLIZE_TIME_MS 1500

extern void init_uart0(void);
extern void UART0_transmit(uint8_t data);

extern void dht11_main(void);

FILE OUTPUT = FDEV_SETUP_STREAM(UART0_transmit, NULL, _FDEV_SETUP_WRITE);

int main(void)
{
	init_uart0();
	
	stdout = &OUTPUT;    
	sei();
	
	while (1)
	{
		dht11_main();
		_delay_ms(STABLIZE_TIME_MS);
	}
	return 0;
}

