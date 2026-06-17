/*
 * 04.UART.c
 *
 * Created: 2026-06-16 오전 9:51:09
 * Author : kccistc
 */ 
#define F_CPU 16000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>

#include "button.h"

void init_timer2(void);

extern void led_main(void);

extern void init_uart0(void);
extern void UART0_transmit(uint8_t data);
extern void process_pc_command(void);

FILE OUTPUT = FDEV_SETUP_STREAM(UART0_transmit, NULL, _FDEV_SETUP_WRITE);

volatile uint32_t ms_count=0;

ISR(TIMER2_OVF_vect)
{
	TCNT2 = 193;
	ms_count++;                 // 1ms count
}

typedef enum STATE { MIN_SEC_CLOCK, SEC_CIRCULAR_CLOCK, 
	DEFAULT_STOP_WATCH, RUN_WATCH, RESET_WATCH };

int main(void)
{	
	init_timer2();
	init_uart0();
	init_led();
	
	stdout = &OUTPUT;         // printf가 동작 할 수 있도록 stdout을 설정
	sei();
	
	while (1)
	{
		process_pc_command();
	}
	return 0;
}

/*
* 1. timer 2 초기화
* 2. 8bit로 1ms 측정하는 timer/counter를 생성
*    2.1 분주비 설정 (256분주)
*        16,000,000Hz / 256 = 62,500Hz
*    2.2 1주기 시간 계산
*        T = 1/f = 1/62,500 = 0.000016sec ( 0.016ms )
*	 2.3 8bit로 count할 시간 계산 (8bit timer overflow)
*        0.000016 x 64 = 0.001024 ( 1.024ms )
*        0.000016 x 63 = 0.001008 ( 1.008ms )
*/
void init_timer2()
{
	TCNT2 = 193;                                        // TCNT193 ~256
	TCCR2 &= ~(1 << CS22 | 1 << CS21 | 1 << CS20);    // RESET
	TCCR2 |= 1 << CS22 | 0 << CS21 | 0 << CS20;       // SET -- 64분주 설정을 위한 세팅
	TIMSK |= 1 << TOIE2;                              // TIMER0 Overflow Interrupt => 256이 되는 순간 오버플로우에 따른 인터럽트 발생
}