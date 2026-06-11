/*
 * led.c
 *
 * Created: 2026-06-10 오후 3:10:22
 *  Author: user
 */ 

#include "led.h"

void init_led(void);
void led_all_on(void);
void led_all_off(void);
void led_shift_left_on(void);
void led_shift_right_on(void);
void led_shift_left_keep_on(void);
void led_shift_right_keep_on(void);
int led_main(void);
void led_flower_on(void);
void led_flower_off(void);

int led_main(void)
{
	led_all_off();
	
	while(1)
	{
		led_shift_left_keep_on();
		_delay_ms(300);
		
		led_all_off();
		_delay_ms(300);
		
		led_shift_right_keep_on();
		_delay_ms(300);
		
		led_all_off();
		_delay_ms(300);
		
		led_flower_on();
		_delay_ms(300);
		
		led_all_off();
		_delay_ms(300);
		
		led_flower_off();
		_delay_ms(300);
	}
	
	return 0;
}

// ---- LED 세팅 초기화 ----
void init_led(void)
{
	DDRA=0xff;  // PORTA 를 출력 모드로 설정
	PORTA=0x00;  // PORTA에 물려있는 led를 all off
}

// ---- LED 모두 점등 ----
void led_all_on(void)
{
	PORTA=0xff;
}

// ---- LED 모두 소등 ----
void led_all_off(void)
{
	PORTA=0x00;
}

// ---- LED 왼쪽부터 하나씩 점등 ----
void led_shift_left_on(void)
{
	static int i = 0;
	
	*(unsigned char *) 0x3B = 1 << i;
	i = (i + 1) % 8;           
	
	_delay_ms(30);
}

// ---- LED 오른쪽부터 하나씩 점등 ----
void led_shift_right_on(void)
{
	static int i = 0;

	*(unsigned char *) 0x3B = 0x80 >> i;
	i = (i + 1) % 8;           
	
	_delay_ms(30);
}

// ---- LED 왼쪽부터 채워서 점등 ----
void led_shift_left_keep_on(void)
{
	static int i = 0;
		
	PORTA |= 1 << i;
	
	i = (i + 1) % 8;      
	
	if(i == LED_NUMBER - 1)
	{
		led_all_off();
	}
	
	_delay_ms(30);
}

// ---- LED 오른쪽부터 채워서 점등 ----
void led_shift_right_keep_on(void)
{
	static int i = 0;
	
	PORTA |= 0x80 >> i;
	i = (i + 1) % 8;      
	
	if(i == LED_NUMBER - 1)
	{
		led_all_off();
	}
	
	_delay_ms(30);
}

// ---- LED flower on ----
void led_flower_on(void)
{
	static int i = 0;

	*(unsigned char *) 0x3B |= 1 << (3-i) | 1 << (4+i);
	i = (i + 1) % 4;      
	
	if(i == LED_NUMBER/2 - 1)
	{
		led_all_off();
	}
	
	_delay_ms(30);
}

// ---- LED flower off ----
void led_flower_off(void)
{
	static int i = 0;
	
	if(i == 0)
	{
		led_all_on();			
		_delay_ms(30);
	}
	else
	{
		*(unsigned char *) 0x3B &= ~(1 << i | 1 << (7-i));	
	}
	
	i = (i + 1) % 5;      
	
	_delay_ms(30);
}
