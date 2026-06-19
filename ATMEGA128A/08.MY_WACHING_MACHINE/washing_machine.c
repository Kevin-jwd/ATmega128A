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

void washing_machine_timer(uint32_t sec_count, uint32_t count, uint32_t display_ms_count);
extern void washing_machine_motor(int mode);
extern void one_circle_display(uint32_t display_ms_count);
extern void display_running_time(uint32_t sec_count);

extern motor_direction;

void washing_machine_main(void);
void machine_ready(void);
void input_time(int state);
void wash(void);
void rinse(void);
void dehydrate(void);

volatile uint32_t display_ms_count = 0;                  // 대기 상태 원형 fnd 디스플레이 카운트
volatile uint32_t ms_count=0;                            // 세탁, 헹굼, 탈수 ms 카운트
volatile uint32_t sec_count=0;                           // 세탁, 헹굼, 탈수 sec 카운트
volatile uint32_t change_sec_count = 0;

uint32_t time_setting[3] = {60, 60, 60};        // 세탁, 헹굼, 탈수 sec 배열	
uint8_t is_displaying = 0;                               // 현재 출력 상태 확인용 비트
uint8_t is_direction_changed = 1;

// 세탁기 상태 
volatile uint32_t state = 0;                              // 현재 상태 
volatile uint32_t is_machine_running = 0;                 // 세탁, 헹굼, 탈수 감지 플래그

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
	
	// 세탁, 헹굼, 탈수 중이라면 ms_count 카운트
    if(is_machine_running == 1)
    {
	    ms_count++;
	    if(ms_count >= 1000)
	    {
		    ms_count = 0;
		    sec_count++;
			change_sec_count++;
	    }
		if(change_sec_count >= 3)
		{
			motor_direction = !motor_direction;
			change_sec_count = 0;
			is_direction_changed = 1;
		}
    }
}

void washing_machine_main(void)
{
	while(1)
	{
		switch(state)
		{
			// 대기 상태
			case READY:
				machine_ready();
				if(get_button(BUTTON1, BUTTON1PIN)) state = 1;
				else if(get_button(BUTTON0, BUTTON0PIN)) 
				{
					state = 4;
					//turn_off_fnd();
				}
				break;
			// 세탁 시간 입력
			case TWASH:
				input_time(1);
				if(get_button(BUTTON1, BUTTON1PIN)) 
				{
					state = 2;
					turn_off_fnd();
				}
				else if(get_button(BUTTON2, BUTTON2PIN)) 
				{
					is_displaying = 0;
					time_setting[0] += 5;
					turn_off_fnd();
				}
				break;
			// 헹굼 시간 입력
			case TRINSE:
				input_time(2);
				if(get_button(BUTTON1, BUTTON1PIN))
				{
					state = 3;
					turn_off_fnd();
				}
				else if(get_button(BUTTON2, BUTTON2PIN))
				{
					is_displaying = 0;
					time_setting[1] += 5;
					turn_off_fnd();
				}
				break;
			// 탈수 시간 입력
			case TDEHYDRATE:
				input_time(3);
				if(get_button(BUTTON1, BUTTON1PIN)) 
				{
					state = 0;
					time_setting[2] = 60;
					turn_off_fnd();
				}
				else if(get_button(BUTTON2, BUTTON2PIN))
				{
					is_displaying = 0;
					time_setting[2] += 5;
					turn_off_fnd();
				}
				break;
			// 세탁
			case WASH:
				wash();
				if(get_button(BUTTON0, BUTTON0PIN))
				{
					 state = 0;
					 is_displaying = 0;
					 turn_off_fnd();
				}
				break;
			// 헹굼
			case RINSE:
				rinse();
				if(get_button(BUTTON0, BUTTON0PIN))
				{
					state = 0;
					is_displaying = 0;
					turn_off_fnd();
				}
				break;
			// 탈수
			case DEHYDRATE:
				dehydrate();
				if(get_button(BUTTON0, BUTTON0PIN))
				{
					state = 0;
					is_displaying = 0;
					turn_off_fnd();
				}
				break;
		}
	}
}

// ---- 대기 상태 함수 ----
void machine_ready()
{
	one_circle_display(display_ms_count);
		
	is_machine_running = 0;
	washing_machine_motor(0);
	
	PORTA = 0x00;
	sec_count = 0;
	
	if(is_displaying != (1 << READY))
	{
		printf("READY\n");
		is_displaying = 1 << READY;
	}
}

// ---- 시간 입력 함수 ----
void input_time(int state)
{
	switch(state)
	{
		case 1: 
		if(is_displaying != (1 << TWASH))
		{
			printf("wash time: %d\n", time_setting[0]);
			is_displaying = 1 << TWASH;
		}
		break;
		case 2:
		if(is_displaying != (1 << TRINSE))
		{
			printf("hydrate time: %d\n", time_setting[1]);
			is_displaying = 1 << TRINSE;
		}
		break;
		case 3:
		if(is_displaying != (1 << TDEHYDRATE))
		{
			printf("dehydrate time: %d\n", time_setting[2]);
			is_displaying = 1 << TDEHYDRATE;
		}
		break;
	}
	display_running_time(time_setting[state-1]);
}

// ---- 세탁 함수
void wash()
{
	PORTA |= 1 << 0;
	is_machine_running = 1;
	
	if(is_displaying != (1 << WASH))
	{
		printf("wash\n");
		is_displaying = 1 << WASH;
	}
	
	washing_machine_timer(sec_count, time_setting[0], display_ms_count);
	washing_machine_motor(WASH);
	
	// 시간 종료
	if(sec_count >= time_setting[0])
	{
		sec_count = 0;
		state++;
		led_all_off();
		turn_off_fnd();
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
	washing_machine_timer(sec_count, time_setting[1], display_ms_count);
	washing_machine_motor(RINSE);
	// 시간 종료
	if(sec_count >= time_setting[1])
	{
		sec_count = 0;
		state++;
		led_all_off();
		turn_off_fnd();
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

	washing_machine_timer(sec_count, time_setting[2], display_ms_count);
	washing_machine_motor(DEHYDRATE);
	// 시간 종료
	if(sec_count >= time_setting[2])
	{
		sec_count = 0;
		state = 0;
		led_all_off();
		washing_machine_motor(0);
		turn_off_fnd();
	}
}