/*
 * 08.MY_WACHING_MACHINE.c
 *
 * Created: 2026-06-18 오후 4:04:54
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

#include "button.h"

extern void init_led(void);

extern void init_button(void);

extern void init_fnd(void);

extern void init_timer3_pwm(void);
extern void init_motor_driver(void);
extern void dcmotor_pwm_control_main(void);

extern void UART0_transmit(uint8_t data);
extern void init_uart0(void);
extern void process_pc_command(void);
extern void init_ultrasonic(void);
extern void process_ultrasonic(void);
extern void led_on_by_distance(int distance);

extern void washing_machine_main(void);

FILE OUTPUT = FDEV_SETUP_STREAM(UART0_transmit, NULL, _FDEV_SETUP_WRITE);

int main(void)
{	
	init_led();
	init_button();
	init_fnd();
	init_timer0();
	init_uart0();
	init_motor_driver();
	init_timer3_pwm();
	
	stdout = &OUTPUT;         // printf가 동작 할 수 있도록 stdout을 설정
	sei();

	while (1)
	{
		washing_machine_main();
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
