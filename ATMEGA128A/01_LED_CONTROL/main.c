/*
 * 01_LED_CONTROL.c
 *
 * Created: 2026-06-10 오전 10:20:29
 * Author : kccistc
 */ 

#define F_CPU 16000000UL      // 16M Hz
#include <avr/io.h>           // PORTA PORTD 등의 I/O register 등 포함
#include <util/delay.h>       // _delay_ms, _delay_us 등의 함수 포함
#include "button.h"

extern void init_led();       // extern : 함수가 다른 파일에 존재하는 것을 complier에 알림
extern void led_all_on();
extern void led_all_off();
extern void led_right_on();
extern void led_left_on();
extern void led_odd_on();
extern void led_even_on();
extern void init_button();
extern int get_button(int button_number, int button_pin);

// 6/10 과제 2
#if 0
int main(void)
{	
	init_button();
	init_led();
	
	int button_cnt = 0;
	
	led_all_off();     // 초기화
	
	while (1)
	{
		// RESET - 버튼 눌림이 감지되면 ( 3, 6 )
		if(get_button(BUTTON3, BUTTON3PIN))
		{
			led_all_off();
			button_cnt = 0;			
		}
		
		// 버튼 눌림이 감지되면 ( 1, 4 )
		if(get_button(BUTTON1, BUTTON1PIN))
		{
			button_cnt ++;
			switch(button_cnt)
			{
				case 0:
				led_all_off();
				break;
				case 1:
				led_odd_on();
				break;
				case 2:
				led_even_on();
				break;
				case 3:
				led_all_on();
				button_cnt = -1;
				break;
			}
		}
	}
	return 0;
}
#endif

// 6/10 과제 1
#if 1
int main(void)
{
	init_button();
	init_led();
	
	int button_cnt = 0;
	
	led_all_off();
	
	while (1)
	{
		// RESET - 버튼 눌림이 감지되면 ( 3, 6 )
		if(get_button(BUTTON3, BUTTON3PIN))
		{
			led_all_off();
			button_cnt = 0;
		}		
		// 버튼 눌림이 감지되면 ( 0, 3 )
		if(get_button(BUTTON0, BUTTON0PIN))
		{
			button_cnt ++;
			switch(button_cnt)
			{
				case 0:
				led_all_off();
				break;
				case 1:
				led_all_on();
				break;
				case 2:
				led_right_on();
				break;
				case 3:
				led_left_on();
				button_cnt = -1;
				break;
			}
		}
	}
	return 0;
}
#endif


#if 0
int main(void)
{
	init_led();

    while (1)  
    {
		PORTA = 0b11111111;    // PORTA 7-0 전부 출력
		_delay_ms(1000);       // 1초 유지
		
		PORTA = 0b00000000;    // PORTA 7-0 전부 출력 X
		_delay_ms(1000);       // 1초 유지
		
		//if(PIND&0b00001000) PORTA = 0b00000011;       // PORTD3 == 1 이면 PORTA0-1 점등
		//if(PIND&0b00010000) PORTA = 0b00001100;       // PORTD4 == 1 이면 PORTA2-3 점등
		//if(PIND&0b00100000) PORTA = 0b00110000;       // PORTD5 == 1 이면 PORTA4-5 점등
		//if(PIND&0b01000000) PORTA = 0b11000000;	       // PORTD6 == 1 이면 PORTA6-7 점등
    }
	return 0;
}
#endif