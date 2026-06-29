#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>    // sei 등 함수
#include <stdio.h>

extern void init_uart0(void);
extern void UART0_transmit(uint8_t data);
extern void dht11_main(void);
extern void ds1302_main(void);   //

FILE OUTPUT = FDEV_SETUP_STREAM(UART0_transmit, NULL, _FDEV_SETUP_WRITE);

int main(void)
{
	init_uart0();
	stdout = &OUTPUT;   // printf가 동작 할 수 있도록 stdout을 설정
	
	ds1302_main();   //
	
	while (1)
	{
		dht11_main();
		_delay_ms(1500);  // DHT11 안정화 시간
		
	}
}
