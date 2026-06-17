/*
 * 05.ULTRASONIC.c
 *
 * Created: 2026-06-167 오후 13:30:09
 * Author : kccistc
 */ 
#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>

#include "button.h"

extern void led_main(void);

extern void init_uart0(void);
extern void UART0_transmit(uint8_t data);
extern void process_pc_command(void);
extern void init_ultrasonic(void);
extern void process_ultrasonic(void);
extern void led_on_by_distance(int distance);

void init_timer0(void);

FILE OUTPUT = FDEV_SETUP_STREAM(UART0_transmit, NULL, _FDEV_SETUP_WRITE);

volatile uint32_t ms_count=0;
volatile int ultrasonic_check_time = 0;

ISR(TIMER0_OVF_vect)            // 벡터 번호 17
{
	TCNT0 = 6;
	ms_count++;                 // 1ms count
	ultrasonic_check_time++;
}

typedef enum STATE { MIN_SEC_CLOCK, SEC_CIRCULAR_CLOCK, 
	DEFAULT_STOP_WATCH, RUN_WATCH, RESET_WATCH };

int main(void)
{	
	init_led();
	init_timer0();
	init_uart0();
	init_ultrasonic();
	
	int distance = 0;
	
	stdout = &OUTPUT;         // printf가 동작 할 수 있도록 stdout을 설정
	sei();
	
	while (1)
	{
		process_ultrasonic();
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