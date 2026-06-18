/*
 * washing_machine.c
 *
 * Created: 2026-06-18 오후 4:17:29
 *  Author: kccistc
 */ 
#include <avr/io.h>
#include <avr/interrupt.h>
#include "button.h"
#include "washing_machine.h"

extern void washing_machine_timer(int sec_count);
extern void washing_machine_motor(int mode);
extern void one_circle_display(int display_ms_count);
extern void process_washing_time(void);

void washing_machine_main(void);
void machine_ready(void);
void input_time(int state);
void wash(void);
void rinse(void);
void dehydrate(void);

volatile uint32_t display_ms_count = 0;
volatile uint32_t ms_count=0; 
volatile uint32_t sec_count=0;
volatile int ultrasonic_check_time = 0;
volatile uint32_t time_setting[3] = {5, 5, 5};
	
uint8_t is_displaying = 0;

// 세탁기 상태 
volatile uint32_t state = 0;
volatile uint32_t is_machine_running = 0;

ISR(TIMER0_OVF_vect)            // 벡터 번호 17
{
    TCNT0 = 6;
    static uint16_t display_tick = 0;  

    display_tick++;
    if(display_tick >= 300)
    {
	    display_tick = 0;
	    display_ms_count = (display_ms_count + 1) % 6;
    }
    
    if(is_machine_running == 1)
    {
	    ms_count++;
	    if(ms_count >= 1000)
	    {
		    ms_count = 0;
		    sec_count++;
	    }
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
				process_washing_time();
				if(get_button(BUTTON1, BUTTON1PIN)) state = 2;
				else if(get_button(BUTTON2, BUTTON2PIN)) is_displaying = 0;
				break;
			case TRINSE:
				input_time(2);
				process_washing_time();
				if(get_button(BUTTON1, BUTTON1PIN)) state = 3;
				else if(get_button(BUTTON2, BUTTON2PIN)) is_displaying = 0;
				break;
			case TDEHYDRATE:
				input_time(3);
				process_washing_time();
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
			case RINSE:
				rinse();
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
		if(is_displaying != (1 << TRINSE))
		{
			printf("hydrate time:\n ");
			is_displaying = 1 << TRINSE;
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
	if(sec_count >= time_setting[0])
	{
		sec_count = 0;
		state++;
		led_all_off();
	}
}

void rinse()
{
	PORTA |= 1 << 1;
	
	if(is_displaying != (1 << RINSE))
	{
		printf("hydrate\n");
		is_displaying = 1 << RINSE;
	}
	washing_machine_timer(sec_count);
	washing_machine_motor(RINSE);
	// 시간 종료
	if(sec_count >= time_setting[1])
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
	if(sec_count >= time_setting[2])
	{
		sec_count = 0;
		state = 0;
		led_all_off();
		washing_machine_motor(0);
	}
}