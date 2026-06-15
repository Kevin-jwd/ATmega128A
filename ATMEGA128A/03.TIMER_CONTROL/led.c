/*
 * led.c
 *
 * Created: 2026-06-10 오후 3:10:22
 *  Author: user
 */ 

#include "led.h"

#define FUNC_NUM 6

int led_main(void);
void init_led(void);
void led_all_on(void);
void led_all_off(void);
void led_shift_left_on(void);
void led_shift_right_on(void);
void led_shift_left_keep_on(void);
void led_shift_right_keep_on(void);
void led_flower_on(void);
void led_flower_off(void);

extern volatile uint32_t ms_count;    // volatile 최적화 방지

int func_state = 0;

void (*fp[])() =
{
	led_shift_left_on, 
	led_shift_right_on, 
	led_shift_left_keep_on, 
	led_shift_right_keep_on, 
	led_flower_on, 
	led_flower_off
};

int led_main(void)
{
	uint8_t led_toggle = 0;
	init_led();
	led_all_off();
	
	while(1)
	{
		fp[func_state]();
	}
	
	return 0;
}

void init_led(void)
{
	DDRA=0xff;  // PORTA 를 출력 모드로 설정
	PORTA=0x00;  // PORTA에 물려있는 led를 all off
}

void led_all_on(void)
{
	PORTA=0xff;
}

void led_all_off(void)
{
	PORTA=0x00;
}

void led_shift_left_on(void)
{
	static int i = 0;
	
	if(ms_count >= 100)
	{
		ms_count = 0;
		*(unsigned char *) 0x3B = 1 << i;		
		
		if((i = (i + 1) % 8) == 0)
		{
			func_state = (func_state+1) % FUNC_NUM;
		}
	}
}

void led_shift_right_on(void)
{
	static int i = 0;

	if(ms_count >= 100)
	{
		ms_count = 0;
		*(unsigned char *) 0x3B = 0x80 >> i;
			
		if((i = (i + 1) % 8) == 0)
		{
			func_state = (func_state+1) % FUNC_NUM;
		}
	}
}

void led_shift_left_keep_on(void)
{
	static int i = 0;
	
	if(ms_count >= 100)
	{
		ms_count = 0;
	    PORTA |= (1 << i);    
			
		if((i = (i + 1) % 8) == 0)
		{
			func_state = (func_state+1) % FUNC_NUM;
			led_all_off();
		}
	}
}

void led_shift_right_keep_on(void)
{
	static int i = 0;
		
	if(ms_count >= 100)
	{
		ms_count = 0;
		PORTA |= (0x80 >> i);      
			
		if((i = (i + 1) % 8) == 0)
		{
			func_state = (func_state+1) % FUNC_NUM;
			led_all_off();
		}
	}
}

void led_flower_on(void)
{
	static int i = 0;
	
	if(ms_count >= 100)
	{
		ms_count = 0;
		PORTA |= 1 << (3-i) | 1 << (4+i);
		
		if((i = (i + 1) % 8) == 0)
		{
			func_state = (func_state+1) % FUNC_NUM;
		}
	}
}

void led_flower_off(void)
{
	static int i = 0;
	
	if(i == 0)
	{
		led_all_on();
	}
	
	if(ms_count >= 100)
	{
		ms_count = 0;
		PORTA &= ~(1 << i | 1 << (7-i));
		
		if((i = (i + 1) % 4) == 0)
		{
			func_state = (func_state+1) % FUNC_NUM;
		}
	}
}
