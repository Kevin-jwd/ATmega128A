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
	
	*(unsigned char *) 0x3B = 1 << i;
	_delay_ms(30);
	
	i = (i + 1) % 8;           // 다음 인덱스 계산
#if 1
	// 30 * 8 = 240ms 소요
	for(int i =0;i<8;i++)
	{
		PORTA = (1 << i);      // PORTA : 0x1B
		_delay_ms(30);
	}
#else      // original
	// 30 * 8 = 240ms 소요 
	for(int i =0;i<8;i++)
	{
		PORTA = (1 << i);      // PORTA : 0x1B
		_delay_ms(30);
	}
#endif
}

void led_shift_right_on(void)
{
#if 1
	static int i = 0;

	*(unsigned char *) 0x3B = 0x80 >> i;
	_delay_ms(30);
	
	i = (i + 1) % 8;           // 다음 인덱스 계산
#else
	for(int i =7;i>-1;i--)
	{
		PORTA = (1 << i);
		_delay_ms(30);
	}
#endif
}

void led_shift_left_keep_on(void)
{
	for(int i =0;i<8;i++)
	{
	    PORTA |= (1 << i);    
		 // *(unsigned char *)0x3B = 1 << i;    // PORTA : 0x3B;
		_delay_ms(300);
	}
}

void led_shift_right_keep_on(void)
{
	for(int i =0;i<8;i++)
	{
		 PORTA |= (0x80 >> i);      
		 // *(unsigned char *)0x3B = 0x80 >> i;    // PORTA : 0x1B
		_delay_ms(300);
	}
}

void led_flower_on(void)
{
	for(int i =0;i<4;i++)
	{
		PORTA |= (1 << (3-i));
		PORTA |= (1 << (4+i));	
		// PORTA |= (0x18 << i) | (0x18 >> i);		
		// PORTA |= 0x18 << i | 0x18 >> i;		
		_delay_ms(300);
	}	
}

void led_flower_off(void)
{
	led_all_on();
	_delay_ms(300);
	
	for(int i =0;i<4;i++)
	{
		PORTA &= ~(1 << i);
		PORTA &= ~(1 << (7-i));
		_delay_ms(300);
	}	
}
