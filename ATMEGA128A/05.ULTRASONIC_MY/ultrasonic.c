/*
 * ultrasonic.c
 *
 * Created: 2026-06-17 오후 1:48:06
 *  Author: kccistc
 */ 

#include "ultrasonic.h"

extern volatile int ultrasonic_check_time;

volatile int distance = 0;
volatile char scm[50];
volatile int falling_edge_detected = 0;

void init_ultrasonic(void);
void make_trigger(void);
void process_ultrasonic(void);

// echo 인터럽트 서비스 루틴
// INT4 : PORT E 4 외부 초음파 센서 상승/하강 에지 검출
// 0x000A
ISR(INT4_vect)      // 벡터 주소 6
{
	// 1. 상승 에지 검출
	if(ECHO_PORT & (1 << ECHO_PIN))
	{
		TCNT1 = 0;
	}
	// 2. 하강 에지 검출
	else
	{
		// TCNT1 : Timer Count 1
		// ex. TCNT1가 10이 들어 있다고 가점
		//     15.625kHz의 1주기 : 46us
		//     0.0000064sec * 10개 ==> 0.00064us (640us)
		//     640us / 58 (1cm 이동 시간) = 11cm 
		// 1sec = 1000000us
		
		distance = TCNT1 * 1000000.0 * 1024 / F_CPU;
		
		// 소요 시간 -> cm 
		sprintf(scm, "dis: %dcm\n", distance / 58);
		led_on_by_distance(distance / 58);
		falling_edge_detected = 1;
	}
}

// ---- 초음파 센서 초기화 함수 ----
void init_ultrasonic(void)
{
	TRIGGER_DDR |= 1 << TRIGGER_PIN;
	ECHO_DDR &= ~(1 << ECHO_PIN);
	
	// External Interrupt Control Register B
	// EICRA -> INT0 ~ INT3
	// EICRB -> INT4 ~ INT7
	EICRB |= 0 << ISC41 | 1 << ISC40;           // 01 : INTn 핀의 입력값이 상승 에지 또는 하강 에지에서 비동기적으로 인터럽트 요청 발생 (p.289)
	
	// 16bit timer/counter 1번 사용
	// timer int 사용 x
	// 표시할 수 있는 최댓값 65535 (0xffff)
	// 16MHz / 1024(분주) : 16000000Hz/1024 => 15625Hz (15.625kHz)
	// T = 1/15625 = 0.000064sec (64us)
	// 분주비 1024 => 1 0 1 (p.318)
	TCCR1B |= 1 << CS12 | 1 << CS10;
	
	// EINT4
	// External Interrupt Mask Register
	EIMSK |= 1 << INT4;         // 외부 INT 4번 허용 (ECHO핀 사용 목적)	
}

// ---- trig 펄스 생성 함수 ----
void make_trigger(void)
{
	TRIGGER_PORT &= ~(1 << TRIGGER_PIN);    // LOW 설정
	_delay_us(1);
	
	TRIGGER_PORT |= (1 << TRIGGER_PIN);     // HIGH 설정
	_delay_us(15);                          // 규격은 10us이나 reduancy로 인해 15us 설정
	
	TRIGGER_PORT &= ~(1 << TRIGGER_PIN);    // LOW 설정
	_delay_us(10);
}

 
// ---- 1초마다 trig 펄스 생성 함수 ----
void process_ultrasonic(void)
{
	if(falling_edge_detected == 1)
	{
		falling_edge_detected = 0;
		ultrasonic_check_time = 0;
		
		printf("%s", scm);
		make_trigger();	
	}
}