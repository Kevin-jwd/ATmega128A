/*
 * washing_machine.c
 *
 * Created: 2026-06-18 오후 4:17:29
 *  Author: kccistc
 */ 
#include <avr/io.h>
#include <avr/interrupt.h>
#include "button.h"

extern void washing_machine_timer(int sec_count);
extern void washing_machine_motor(int mode);
extern void one_circle_display(int display_ms_count);

void washing_machine_main(void);
void machine_ready(void);
void input_time(int state);
void wash(void);
void hydrate(void);
void dehydrate(void);

volatile uint32_t display_ms_count = 0;
volatile uint32_t ms_count=0; 
volatile uint32_t sec_count=0;
volatile int ultrasonic_check_time = 0;

uint8_t is_displaying = 0;

// 세탁기 상태 
volatile uint32_t state = 0;
volatile uint32_t is_machine_running = 0;

// 0: 대기, 1: 세탁 시간 설정, 2: 헹굼 시간 설정, 3: 탈수 시간 설정, 4: 세탁, 5: 헹굼, 6: 탈수
typedef enum STATE { READY, TWASH, THYDRATE, TDEHYDRATE, WASH, HYDRATE, DEHYDRATE};

ISR(TIMER0_OVF_vect)            // 벡터 번호 17
{
	if(is_machine_running == 1)
	{
		TCNT0 = 6;
		ms_count++;
	}
	if(ms_count >= 300)
	{
		display_ms_count = (display_ms_count + 1) % 6;
	}
	if(ms_count >= 1000)
	{
		ms_count = 0;
		sec_count++;
	}
}

void washing_machine_main(void)
{
	while(1)
	{
		switch(state)
		{
			case READY:
				machine_ready();
				if(get_button(BUTTON1, BUTTON1PIN)) state = 1;
				else if(get_button(BUTTON0, BUTTON0PIN)) state = 4;
				break;
			case TWASH:
				input_time(1);
				if(get_button(BUTTON1, BUTTON1PIN)) state = 2;
				else if(get_button(BUTTON2, BUTTON2PIN)) is_displaying = 0;
				break;
			case THYDRATE:
				input_time(2);
				if(get_button(BUTTON1, BUTTON1PIN)) state = 3;
				else if(get_button(BUTTON2, BUTTON2PIN)) is_displaying = 0;
				break;
			case TDEHYDRATE:
				input_time(3);
				if(get_button(BUTTON1, BUTTON1PIN)) state = 0;
				else if(get_button(BUTTON2, BUTTON2PIN)) is_displaying = 0;
				break;
			case WASH:
				wash();
				if(get_button(BUTTON0, BUTTON0PIN))
				{
					 state = 0;
					 is_displaying = 0;
				}
				break;
			case HYDRATE:
				hydrate();
				if(get_button(BUTTON0, BUTTON0PIN))
				{
					state = 0;
					is_displaying = 0;
				}
				break;
			case DEHYDRATE:
				dehydrate();
				if(get_button(BUTTON0, BUTTON0PIN))
				{
					state = 0;
					is_displaying = 0;
				}
				break;
		}
	}
}

void machine_ready()
{
	is_machine_running = 0;
	
	PORTA = 0x00;
	sec_count = 0;
	turn_off_fnd();
	
	if(is_displaying != (1 << READY))
	{
		printf("READY\n");
		is_displaying = 1 << READY;
	}
}

void input_time(int state)
{
	switch(state)
	{
		case 1: 
		if(is_displaying != (1 << TWASH))
		{
			printf("wash time:\n");
			is_displaying = 1 << TWASH;
		}
		break;
		case 2:
		if(is_displaying != (1 << THYDRATE))
		{
			printf("hydrate time:\n ");
			is_displaying = 1 << THYDRATE;
		}
		break;
		case 3:
		if(is_displaying != (1 << TDEHYDRATE))
		{
			printf("dehydrate time:\n");
			is_displaying = 1 << TDEHYDRATE;
		}
		break;
	}
}

void get_time(int state)
{
	;
}

void wash()
{
	one_circle_display(display_ms_count);
	
	PORTA |= 1 << 0;
	is_machine_running = 1;
	
	if(is_displaying != (1 << WASH))
	{
		printf("wash\n");
		is_displaying = 1 << WASH;
	}
	
	washing_machine_timer(sec_count);
	washing_machine_motor(WASH);
	// 시간 종료
	if(sec_count >= 5)
	{
		sec_count = 0;
		state++;
		led_all_off();
	}
}

void hydrate()
{
	PORTA |= 1 << 1;
	
	if(is_displaying != (1 << HYDRATE))
	{
		printf("hydrate\n");
		is_displaying = 1 << HYDRATE;
	}
	washing_machine_timer(sec_count);
	washing_machine_motor(HYDRATE);
	// 시간 종료
	if(sec_count >= 5)
	{
		sec_count = 0;
		state++;
		led_all_off();
	}
}

void dehydrate()
{
	PORTA |= 1 << 2;

	if(is_displaying != (1 << DEHYDRATE))
	{
		printf("dehydrate\n");
		is_displaying = 1 << DEHYDRATE;
	}

	washing_machine_timer(sec_count);
	washing_machine_motor(6);
	// 시간 종료
	if(sec_count >= 5)
	{
		sec_count = 0;
		state = 0;
		led_all_off();
		washing_machine_motor(0);
	}
}