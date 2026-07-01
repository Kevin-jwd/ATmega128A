/*
 * 13.RTC_LCD_CALC.c
 *
 * Created: 2026-07-01 오전 9:57:30
 * Author : kccistc
 */ 
#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include "lcd.h"

void init_timer0(void);
extern void init_button(void);

extern void init_keypad(void);

extern void init_uart0(void);
extern void UART0_transmit(uint8_t data);

extern void init_lcd(void);
extern int lcd_main(void);
extern void lcd_timer(void);

extern void init_i2c(void);
extern void init_time(void);
extern void read_time(void);

volatile int ms_count = 0;

FILE OUTPUT = FDEV_SETUP_STREAM(UART0_transmit, NULL, _FDEV_SETUP_WRITE);

ISR(TIMER0_OVF_vect)            // 벡터 번호 17
{
	TCNT0 = 6;
	ms_count++;
}

int main(void)
{
	init_button();
	init_keypad();
	init_uart0();
	init_i2c();
	init_time();
	init_timer0();
	init_lcd();
	
	stdout = &OUTPUT;         // printf가 동작 할 수 있도록 stdout을 설정
	sei();
    while (1) 
    {
		read_time();
		lcd_timer();
		_delay_ms(1000);
    }
	return 0;
}

void init_timer0()
{
	TCNT0 = 6;                                        // TCNT0 6~256
	TCCR0 &= ~(1 << CS02 | 1 << CS01 | 1 << CS00);    // RESET
	TCCR0 |= 1 << CS02 | 0 << CS01 | 0 << CS00;       // SET -- 64분주 설정을 위한 세팅
	TIMSK |= 1 << TOIE0;                              // TIMER0 Overflow Interrupt => 256이 되는 순간 오버플로우에 따른 인터럽트 발생
}
