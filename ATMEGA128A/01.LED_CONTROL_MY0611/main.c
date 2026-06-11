/*
 * 01.LED_CONTROL.c
 *
 * Created: 2026-06-10 오전 10:20:27
 * Author : user
 */ 

#define F_CPU 16000000UL   // 16MHz
#include <avr/io.h>  // PORTA PORTD 등의 I/O register들이 들어 있다. 
#include <util/delay.h>   // _delay_ms _delay_us 등의 함수 가 들어 있다. 

#include "button.h"

extern void init_led(void);  
extern void init_button(void);
extern int get_button(int button_num, int button_pin);
extern int led_main(void);
extern void led_all_on(void);
extern void led_all_off(void);
extern void led_shift_left_on(void);
extern void led_shift_right_on(void);
extern void led_shift_left_keep_on(void);
extern void led_shift_right_keep_on(void);
extern void led_flower_on(void);
extern void led_flower_off(void);

typedef enum STATE {LED_ALL_OFF, LED_ALL_ON, LED_SHIFT_LEFT_ON, LED_SHIFT_RIGHT_ON,
	LED_SHIFT_LEFT_KEEP_ON, LED_SHIFT_RIGHT_KEEP_ON, FLOWER_ON, FLOWER_OFF };

#if 1
int main(void)
{
	int button0_cnt = 0;   // 버튼 눌림 카운트
	
	init_button();
	init_led();
	
	while (1)
	{
		if (get_button(BUTTON0, BUTTON0PIN))
		{
			if(button0_cnt == 7) button0_cnt = 0;
			else	button0_cnt++;
		}
		
		switch(button0_cnt)
		{
			case LED_ALL_OFF :
			led_all_off();
			break;
			case LED_ALL_ON :
			led_all_on();
			break;
			case LED_SHIFT_LEFT_ON :
			led_shift_left_on();
			break;
			case LED_SHIFT_RIGHT_ON :
			led_shift_right_on();
			break;
			case LED_SHIFT_LEFT_KEEP_ON :
			led_shift_left_keep_on();
			break;
			case LED_SHIFT_RIGHT_KEEP_ON :
			led_shift_right_keep_on();
			break;
			case FLOWER_ON :
			led_flower_on();
			break;
			case FLOWER_OFF :
			led_flower_off();
			break;
			default :
			led_all_off();
		}
	}
}
#endif

#if 0
int main(void)
{
	DDRA = 0b11111111;    // PORTA에 LED가 8개 연결 되어 있으므로 all 1(출력)으로 설정 한다.
	                      // DDR(Data Direction Register) 방향 설정
						  // 1: 출력 0:입력 
    while (1) 
    {
		PORTA = 0b11111111;   // all on
		_delay_ms(1000);   // 1초 유지  1000ms : 1초 500ms : 0.5초
		PORTA = 0b00000000;  // all off
		_delay_ms(1000);    // 1초 sleep
    }
}
#endif 
