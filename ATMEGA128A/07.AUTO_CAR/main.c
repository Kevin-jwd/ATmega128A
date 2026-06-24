/*
 * 07.DCMOTOR_PWN_CONTROL.c
 *
 * Created: 2026-06-18 오후 2:03:49
 * Author : kccistc
 */ 

/*
* PE5 : PWM 신호
* PF6 : IN1 (모터 드라이버)
* PF7 : IN2 (모터 드라이버)
* (PF6, PF7) : (0, 1) 역회전 / (1, 0) 정회전 / (1, 1) STOP
*/

#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>

#include "define.h"

extern void init_led(void);

extern void init_button(void);

extern void init_timer1_pwm(void);
extern void init_motor_driver(void);

extern void stop(void);
extern void forward(int speed);
extern void backward(int speed);
extern void turn_left(int speed);
extern void turn_right(int speed);

extern void init_uart1(void);
extern void init_uart0(void);
extern void UART0_transmit(uint8_t data);

extern void led_on_by_distance(int distance);

extern volatile uint8_t bt_data;

void manual_mode(void);
void auto_mode(void);
void chekc_auto_mode(void);
void chekc_distance(void);

volatile uint32_t ms_count=0;
volatile int ultrasonic_check_time = 0;

FILE OUTPUT = FDEV_SETUP_STREAM(UART0_transmit, NULL, _FDEV_SETUP_WRITE);

ISR(TIMER0_OVF_vect)            // 벡터 번호 17
{
	TCNT0 = 6;
	ms_count++;                 // 1ms count
	ultrasonic_check_time++;
}

int func_state = MANUAL_MODE;

void (*pfunc[])() = 
{
	MANUAL_MODE,
	AUTO_MODE,
	CHECK_AUTO_MODE,
	CHECK_DISTANCE
};

void manual_mode(void)
{
	switch(bt_data)
	{
		// STOP
		case 's':
		case 'S':
			stop();
			break;
		// FORWARD
		case 'f':
		case 'F':
			forward(500);      // 4us x 500 = 0.002s (2ms)
			break;
		// BACKWARD
		case 'b':
		case 'B':
			backward(500);
			break;
		// LEFT
		case 'l':
		case 'L':
			turn_left(700);     // 4us x 700 = 0.0028s (28ms)
			break;
		// RIGHT
		case 'r':
		case 'R':
			turn_right(700);
			break;
		default:
			break;
	}
}

void auto_mode(void)
{
	
}

void chekc_auto_mode(void)
{
	
}

void chekc_distance(void)
{
	
}


int main(void)
{	
	init_led();
	init_button();
	init_timer0();
	init_timer1_pwm();
	init_motor_driver();
	
	init_uart1();
	init_uart0();
	
	
	stdout = &OUTPUT;         // printf가 동작 할 수 있도록 stdout을 설정
	sei();
	
	while (1)
	{
		pfunc[func_state]();
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

