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

int led_main(void)
{
	init_led();
	led_all_off();
	
	while(1)
	{
		;
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
		i = (i + 1) % 8;
		//if((i = (i + 1) % 8) == 0)
		//{
			//func_state = (func_state+1) % FUNC_NUM;
		//}
	}
}

void led_shift_right_on(void)
{
	static int i = 0;

	if(ms_count >= 100)
	{
		ms_count = 0;
		*(unsigned char *) 0x3B = 0x80 >> i;
		i = (i + 1) % 8;
		//if((i = (i + 1) % 8) == 0)
		//{
			//func_state = (func_state+1) % FUNC_NUM;
		//}
	}
}

void led_shift_left_keep_on(void)
{
	static int i = 0;

	if(ms_count >= 100)
	{
		ms_count = 0;
		if(i == 0)
		{
			led_all_off();
		}
		else
		{
			PORTA |= (1 << i-1);    	
		}
		
		i = (i + 1) % 9;

		//if((i = (i + 1) % 8) == 0)
		//{
			//func_state = (func_state+1) % FUNC_NUM;
			//led_all_off();
		//}
	}
}

void led_shift_right_keep_on(void)
{
	static int i = 0;
	
	if(ms_count >= 100)
	{
		ms_count = 0;
		if(i == 0)
		{
			led_all_off();
		}
		else
		{
			PORTA |= (0x80 >> i-1);      
		}
	
		i = (i + 1) % 9;
	
		//if((i = (i + 1) % 8) == 0)
		//{
			//func_state = (func_state+1) % FUNC_NUM;
			//led_all_off();
		//}
	}
}

void led_flower_on(void)
{
	static int i = 0;
	
	if(ms_count >= 100)
	{
		ms_count = 0;
		if(i == 0)
		{
			led_all_off();
		}
		else
		{
			PORTA |= 1 << (4-i) | 1 << (3+i);			
		}
		i = (i+1) % 5;
		
		//if((i = (i + 1) % 4) == 0)
		//{
			//func_state = (func_state+1) % FUNC_NUM;
		//}
	}
}

void led_flower_off(void)
{
	static int i = 0;

	if(ms_count >= 100)
	{	
		ms_count = 0;
		if(i == 0)
		{
			led_all_on();			
		}
		else
		{
			PORTA &= ~(1 << i-1 | 1 << (8-i));		
		}

		i = (i+1) % 5;	
		
		//if((i = (i + 1) % 4) == 0)
		//{
			//func_state = (func_state+1) % FUNC_NUM;
		//}
	}
}
