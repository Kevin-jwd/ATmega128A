/*
 * led.c
 *
 * Created: 2026-06-10 오후 3:10:30
 *  Author: kccistc
 */ 
#include "led.h"

void init_led();
void led_all_on();
void led_all_off();

// LED 관련 포트 초기화 및 LED 소등
void init_led()
{
	DDRA = 0xFF;       // PORTA 출력모드  
	                   // Data Direction Register 방향 설정, PORTA 7-0, output : 1, input : 0
	PORTA = 0x00;      // LED 전부 off
}

// LED 전부 켜기
void led_all_on()
{
	PORTA = 0xFF;      // PORTA 7-0 전부 출력
}

// LED 전부 끄기
void led_all_off()
{
	PORTA = 0x00;      // PORTA 7-0 전부 출력 x
}

// LED 오른쪽 절반 켜기
void led_right_on()
{
	PORTA = 0x0F;
}

// LED 왼쪽 절반 켜기
void led_left_on()
{
	PORTA = 0xF0;
}

// LED 홀수 켜기
void led_odd_on()
{
	PORTA = 0x00;
	for(int i = 0;i<LED_NUMBER;i++)
	{
		if(i % 2 == 0)
		{
			PORTA |= (1 << i);			
		}
	}
}

// LED 짝수 켜기
void led_even_on()
{
	PORTA = 0x00;
	for(int i =0;i<LED_NUMBER;i++)
	{
		if(i % 2 == 1)
		{
			PORTA |= (1 << i);
		}
	}
}