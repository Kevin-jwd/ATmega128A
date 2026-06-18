/*
 * 06.PWM_CONTROL.c
 *
 * Created: 2026-06-18 오전 10:23:09
 * Author : kccistc
 */ 

#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>

#define LED_TIME 20

void init_led(void);
void led_all_on(void);
void led_all_off(void);
void turn_on_LED_in_PWM_manner(int dim);

int main(void)
{
    init_led();
	
	int dim = 0;
	int direction = 1;
	
    while (1) 
    {
		turn_on_LED_in_PWM_manner(dim);
		
		dim += direction;
		
		if(dim == 0) direction = 1;
		if(dim == 255) direction = -1;
    }
}

// ---- LED 초기화 함수 ----
void init_led()
{
	DDRA=0xff;  // PORTA 를 출력 모드로 설정
	PORTA=0x00;  // PORTA에 물려있는 led를 all off
}

// ---- LED 모두 점등 함수 ----
void led_all_on()
{
	PORTA = 0xff;
}

// ---- LED 모두 소등 함수 ----
void led_all_off()
{
	PORTA = 0x00;
}

// ---- PWM 신호 생성 함수 ----
void turn_on_LED_in_PWM_manner(int dim)
{
	
	int i;
	led_all_on();
	
	for(i = 0;i < 256;i++)
	{	
		if(i > dim) 
		{
			led_all_off();
		}
		_delay_us(LED_TIME);
	}
}