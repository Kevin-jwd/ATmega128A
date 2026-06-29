/*
 * 11.KEYPAD.c
 *
 * Created: 2026-06-29 오후 1:42:36
 * Author : kccistc
 */ 
#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>

#include "queue.h"

extern void    init_uart0(void);
extern void    UART0_transmit(uint8_t data);

extern void    init_keypad(void);
extern uint8_t scan_keypad(void);

extern void init_queue(void);

void           init_timer0(void);

volatile uint32_t keypad_count=0;

FILE OUTPUT = FDEV_SETUP_STREAM(UART0_transmit, NULL, _FDEV_SETUP_WRITE);

ISR(TIMER0_OVF_vect)            // 벡터 번호 17
{
	volatile uint8_t keypad_data = 0;
	
	TCNT0 = 6;
	if(++keypad_count >= 60)
	{
		keypad_count = 0;
		if(keypad_data = scan_keypad())     // keypad를 확인해서 눌린 것이 있는지 확인
		{
			insert_queue(keypad_data);
		}
		
	}
}

int main(void)
{	
	uint8_t key_value = 0;
	
	init_timer0();
	init_uart0();
	init_keypad();
	queue_init();
	
	stdout = &OUTPUT;         // printf가 동작 할 수 있도록 stdout을 설정
	sei();
	
	while (1)
	{
		if(queue_empty() != TRUE)
		{
			key_value = read_queue();
			printf("%c", key_value);
		}
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

