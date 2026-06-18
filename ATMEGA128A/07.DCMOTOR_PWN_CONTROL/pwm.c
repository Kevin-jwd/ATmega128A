/*
 * pwm.c
 *
 * Created: 2026-06-18 오후 2:22:48
 *  Author: kccistc
 */ 
#include "pwm.h"

extern int get_button(int button_num, int button_pin);

void init_timer3_pwm(void);
void init_motor_driver(void);
void dcmotor_pwm_control_main(void);

/*
* ---- 타이머 3번 PWM 초기화 함수 ----
* 16 bit 3번 timer/counter 사용
* PWM 출력 신호
* ============
* PE3 : OC3A
* PE4 : OC3B -> INT4 사용 (초음파 센서)
* PE5 : OC3C -> DC MOTOR (PWM 출력)
* BTN1 : speed-UP
* BTN2 : speed-DOWN
*/
void init_timer3_pwm()
{
	DDRE |= 1 << 3 | 1 << 5;
	
	// 8bit 고속 PWM 모드
	TCCR3A |= 1 << WGM30;                             // WGMnm -> n=3
	TCCR3B |= 1 << WGM32;
	
	// 비반전 모드
	TCCR3A |= 1 << COM3C1;            // COMnx1, COMnx0 -> OC3C이므로 x=C
	
	// 분주비 설정
	// 분주비 64
	// 16000000Hz / 64 = 250000Hz
	// T = 1/f = 1 / 250000Hz = 0.000004s (4us)
	// 250000Hz에서 256개 펄스 count하면 소요시간 : 1.02ms
	// 250000Hz에서 127개 펄스 count하면 소요시간 : 0.50ms
	// TCNT3 : 0~255(0x00ff) 까지 count한 후 0으로 초기화
	TCCR3B |= 1 << CS31 | 1 << CS30;           // 분주비 64
	
	OCR3C = 0;                                 // OCR (Output Compare Register)
	// ex. OCR3C = 50; Duty 비 = (OCR3C / TOP) x 100 = 50 / 255 x 100 = 19.61%
}

/* ---- 모터 드라이버 초기화 함수 ----
* PE5 : PWM control
* PF6 : IN1 (방향 설정)
* PF7 : IN2
*/
void init_motor_driver()
{
	DDRF |= 1 << 6 | 1 << 7;          // 출력 모드 설정
	PORTF &= ~(1 << 6 | 1 << 7);     // RESET
	PORTF |= 1 << 6;                 // 정회전
}

void dcmotor_pwm_control_main(void)
{
	int start_button = 0;
	int forward = 0;
	
	while(1)
	{
		// start stop
		if(get_button(BUTTON0, BUTTON0PIN))        
		{
			start_button = !start_button;
			if(start_button) OCR3C = 250;
			else OCR3C = 0;
		}
		// speed-up
		else if(get_button(BUTTON1, BUTTON1PIN))
		{
			
			if(OCR3C >= 250) OCR3C = 250;
			else OCR3C += 20;
		}
		// speed-down
		else if(get_button(BUTTON2, BUTTON2PIN))
		{
			if(OCR3C <= 70)	OCR3C = 70;
			else OCR3C -= 20;
		}
		// 방향 전환
		else if(get_button(BUTTON3, BUTTON3PIN))
		{
			forward = !forward;
			PORTF &= ~(1 << 6 | 1 << 7);
			if(forward) PORTF |= 1 << 6;              // 정회전
			else PORTF |= 1 << 7;                     // 역회전
		}
	}
}

