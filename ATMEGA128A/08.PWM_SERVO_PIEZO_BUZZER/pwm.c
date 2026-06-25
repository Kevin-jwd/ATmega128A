/*
 * pwm.c
 *
 * Created: 2026-06-25 오전 9:47:48
 *  Author: kccistc
 */ 
#include "button.h"

#define SERVO_PORT_DDR DDRB
#define SERVO_PORT_PIN 5

int servo_motor_main(void);
void init_timer1_pwm(void);

/*
* ---- 타이머 1번 PWM 초기화 함수 ----
*
* 16 bit 1번 timer/counter 사용
* 
* PWM 출력 신호
* ========================
* PB5 : OC1A : 서보 모터
* 
*/

int servo_motor_main()
{
	// servo motor 에 연결된 PORTB 5 출력으로 설정
	SERVO_PORT_DDR |= 1 << SERVO_PORT_PIN;
	init_timer1_pwm();
	
	// 0 (1ms) --> 90 (1.5ms) --> 180 (2ms)
	while(1)
	{
		// ---- 0도 회전 ----
		// 1ms : 펄스 2000 카운트
		// 20ms (40000) : 1ms (2000)
		OCR1A = 2000;
		_delay_ms(1000);
		
		// ---- 90도 회전 ----
		// 1.5ms : 펄스 3000 카운트
		// tolerance에 따라 1600 설정; 2000 + 1600
		OCR1A = 3600;
		_delay_ms(1000);
		
		// ---- 180도 회전 ----
		// 2ms : 펄스 4000 카운트
		// tolerance에 따라 1600 설정; 2000 + 1600 + 1600
		OCR1A = 5200;
		_delay_ms(1000);
	}
	return 0;
}

void init_timer1_pwm()
{
	// 모드 14 : 16bit 고속 PWM 모드 (1, 1, 1, 0) (WGM13, WGM12, WGM11, WGM10) (TCCR1A, TCCR1B)
	TCCR1A |= 1 << WGM11;                  // TOP -> IC1으로 설정
	TCCR1B |= 1 << WGM13 | 1 << WGM12;       
	
	// 비교 일치 발생 시 OCR1의 출력핀을 LOW로 바뀌고 BOTTOM에서 HIGH로 바뀜
	TCCR1A |= 1 << COM1A1;
	
	// 분주비 설정
	// 분주비 8 ; 16MHz를 갖고 50Hz의 주파수를 바로 만들지 못하기 떄문
	// 16000000Hz / 8 = 2Mhz
	// T = 1/f = 1 / 16000000Hz = 0.0000000625s (4us) x 8 = 0.00000005s (0.5us)
	// T = 1/f = 1 / 2MHz = 0.0000005s (0.5us)
	// 16bit 최대 카운트 65535 (0xFFFF)
	// 0.0000005s x 65535 = 0.0327675s (32.7675ms) 마다 timer overflow interrupt
	// 20ms 길이 (Duty)를 갖는 것을 만든다면
	// 0.0000005 x 40000 = 0.02 sec (20ms)

	TCCR1B &= ~(1 << CS21 | 1 << CS11 | 1 << CS10);        // RESET
	TCCR1B |= 0 << CS21 | 1 << CS11 | 0 << CS10;           // 0, 1, 0 -> 분주비 8
	
	ICR1 = 40000 - 1;       // top값 -> 0.0000005 x 40000 = 20ms
}