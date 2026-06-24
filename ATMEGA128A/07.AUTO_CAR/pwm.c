/*
 * pwm.c
 *
 * Created: 2026-06-18 오후 2:22:48
 *  Author: kccistc
 */ 
#include "pwm.h"

#define MOTOR_PWM_DDR DDRB
#define MOTOR_LEFT_PORT_DDR 5      // OC1A
#define MOTOR_RIGHT_PORT_DDR 6      // OC1B

#define MOTOR_DIRECTION_PORT PORTF
#define MOTOR_DIRECTION_DDR DDRF

void init_timer1_pwm(void);
void init_motor_driver(void);
void stop(void);
void forward(int speed);
void backward(int speed);
void turn_left(int speed);
void turn_right(int speed);

/*
* ---- 타이머 1번 PWM 초기화 함수 ----
* 16 bit 1번 timer/counter 사용
* ========================
* PWM 출력 신호
* ========================
* PB5 : OC1A : 왼쪽 바퀴
* PB6 : OC1B : 오른쪽 바퀴
* BTN0 : AUTO <-> manual
* ========================
* 방향 설정
* ========================
* 1. 왼쪽 바퀴
*     PF0 -- IN1
*     PF1 -- IN2 
* 2. 오른쪽 바퀴
*     PF2 -- IN3
*     PF3 -- IN4
* 
*     IN1/IN3       IN2/IN4
*     =======       =======
*        0             1         : 역회전
*        1             0         : 정회전
*        1             1         : 정지
*/

// ---- 타이머 1 초기화 함수 ----
void init_timer1_pwm()
{
	// 분주비 설정
	// 분주비 64
	// 16000000Hz / 64 = 250000Hz
	// T = 1/f = 1 / 250000Hz = 0.000004s (4us)
	// 250000Hz에서 256개 펄스 count하면 소요시간 : 1.02ms
	// 250000Hz에서 127개 펄스 count하면 소요시간 : 0.50ms
	// 250000Hz에서 0x3ff(1023) 개 펄스 count하면 소요시간 : 4ms
	TCCR1B |= 1 << CS11 | 1 << CS10;           // 분주비 64
	
	//// 초음파 연결
	//DDRE |= 1 << 3 | 1 << 5;
	
	// 모드 14 : 16bit 고속 PWM 모드 (1, 1, 1, 0) (WGM13, WGM12, WGM11, WGM10) (TCCR1A, TCCR1B)
	TCCR1A |= 1 << WGM11;                  // TOP -> IC1으로 설정
	TCCR1B |= 1 << WGM13 | 1 << WGM12;       
	
	
	// 비반전 모드 top : ICR1 비교일치 값 지정 OCR1A OCR1B
	// 비교 일치 발생 시 OCR1, OCR1B의 출력핀을 LOW로 바뀌고 BOTTOM에서 HIGH로 바뀜
	TCCR1A |= 1 << COM1A1;
	TCCR1A |= 1 << COM1B1;
	
	ICR1 = 0x3ff;       // = 1023 (top값)
}

/*
* ---- 모터 드라이버 초기화 함수 ----
* 16 bit 1번 timer/counter 사용
* ========================
* PWM 출력 신호
* ========================
* PB5 : OC1A : 왼쪽 바퀴
* PB6 : OC1B : 오른쪽 바퀴
* BTN0 : AUTO <-> manual
* ========================
* 방향 설정
* ========================
* 1. 왼쪽 바퀴
*     PF0 -- IN1
*     PF1 -- IN2
* 2. 오른쪽 바퀴
*     PF2 -- IN3
*     PF3 -- IN4
*
*     IN1/IN3       IN2/IN4
*     =======       =======
*        0             1         : 역회전
*        1             0         : 정회전
*        1             1         : 정지
*/
void init_motor_driver()
{
	// 1. 출력 모드 설정
	MOTOR_PWM_DDR &= ~(1 << MOTOR_LEFT_PORT_DDR | 1 << MOTOR_RIGHT_PORT_DDR);  // 초기화
	MOTOR_PWM_DDR |= 1 << MOTOR_LEFT_PORT_DDR | 1 << MOTOR_RIGHT_PORT_DDR;
	
	MOTOR_DIRECTION_DDR &= ~(1 << 0 | 1 << 1 | 1 << 2 | 1 << 3);          // 초기화   
	MOTOR_DIRECTION_DDR |= (1 << 0 | 1 << 1 | 1 << 2 | 1 << 3);
	
	// 2. 모터 전진으로 설정
	MOTOR_DIRECTION_PORT &= ~(1 << 0 | 1 << 1 | 1 << 2 | 1 << 3);              // 초기화
	MOTOR_DIRECTION_PORT |= 1 << 2 | 1 << 0;             // 모터를 전진 모드로 IN4 IN3 IN2 IN1 (0101)
}

// ---- 정지 함수 ----
void stop()
{
	MOTOR_DIRECTION_PORT &= ~(1 << 0 | 1 << 1 | 1 << 2 | 1 << 3);              // 초기화
	MOTOR_DIRECTION_PORT |= 1 << 0 | 1 << 1 | 1 << 2 | 1 << 3;             // 모터를 전진 모드로 IN4 IN3 IN2 IN1 (1010)

	OCR1A = OCR1B = 0;                                          // OCR1A : PWM left, OCR1B : PWM right
}

// ---- 전진 함수 ----
void forward(int speed)
{
	MOTOR_DIRECTION_PORT &= ~(1 << 0 | 1 << 1 | 1 << 2 | 1 << 3);              // 초기화
	MOTOR_DIRECTION_PORT |= 1 << 2 | 1 << 0;             // 모터를 전진 모드로 IN4 IN3 IN2 IN1 (0101)
	
	OCR1A = OCR1B = speed;                                       // OCR1A : PWM left, OCR1B : PWM right
}

// ---- 후진 함수 ----
void backward(int speed)
{
	MOTOR_DIRECTION_PORT &= ~(1 << 0 | 1 << 1 | 1 << 2 | 1 << 3);              // 초기화
	MOTOR_DIRECTION_PORT |= 1 << 3 | 1 << 1;             // 모터를 전진 모드로 IN4 IN3 IN2 IN1 (1010)
	
	OCR1A = OCR1B = speed;                                       // OCR1A : PWM left, OCR1B : PWM right
}

// ---- 좌회전 함수 ----
void turn_left(int speed)
{
	MOTOR_DIRECTION_PORT &= ~(1 << 0 | 1 << 1 | 1 << 2 | 1 << 3);              // 초기화
	MOTOR_DIRECTION_PORT |= 1 << 3 | 1 << 1;             // 모터를 전진 모드로 IN4 IN3 IN2 IN1 (1010)
	
	OCR1A = 0;
	OCR1B = speed;                                       // OCR1B : PWM right
}

// ---- 우회전 함수 ----
void turn_right(int speed)
{
	MOTOR_DIRECTION_PORT &= ~(1 << 0 | 1 << 1 | 1 << 2 | 1 << 3);              // 초기화
	MOTOR_DIRECTION_PORT |= 1 << 3 | 1 << 1;             // 모터를 전진 모드로 IN4 IN3 IN2 IN1 (1010)
	
	OCR1B = 0;
	OCR1A = speed;                                       // OCR1A : PWM left
}