/*
 * 12.I2C_M_LOOPBACK.c
 *
 * Created: 2026-06-30 오전 10:46:04
 * Author : kccistc
 */ 
#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>

extern void init_uart0(void);
extern void UART0_transmit(uint8_t data);
extern void i2c_master_main(void);
extern void loopback_test(void);

void init_timer0(void);


volatile int ms_count = 0;

FILE OUTPUT = FDEV_SETUP_STREAM(UART0_transmit, NULL, _FDEV_SETUP_WRITE);

ISR(TIMER0_OVF_vect)            // 벡터 번호 17
{
	TCNT0 = 6;
	ms_count++;
}

int main(void)
{	
	init_timer0();
	init_uart0();
	
	stdout = &OUTPUT;         // printf가 동작 할 수 있도록 stdout을 설정
	sei();

	i2c_master_main();
	
	while (1)
	{
		
	}
	return 0;
}

// ---- timer 0번 초기화 함수 ----
void init_timer0()
{
	TCNT0 = 6;                                        // TCNT0 6~256
	TCCR0 &= ~(1 << CS02 | 1 << CS01 | 1 << CS00);    // RESET
	TCCR0 |= 1 << CS02 | 0 << CS01 | 0 << CS00;       // SET -- 64분주 설정을 위한 세팅
	TIMSK |= 1 << TOIE0;                              // TIMER0 Overflow Interrupt => 256이 되는 순간 오버플로우에 따른 인터럽트 발생
}

