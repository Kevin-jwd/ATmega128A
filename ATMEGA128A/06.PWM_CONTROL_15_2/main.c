/*
 * 06.PWM_CONTROL_15_2.c
 *
 * Created: 2026-06-18 오전 11:39:52
 * Author : kccistc
 */ 
#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>

#define LED_TIME 20

void init_led(void);

int main(void)
{
	init_led();
	
    int dim = 0;
	int direction = 0;
#if 1
// 파형 출력 핀 PB5, PB6 출력으로 설정
	DDRB |= (1 << PB5) | (1 << PB6);
	
	// 타이머/카운터 1번을 8비트 위상 교정 PWM 모드로 설정
	TCCR1A |= (1 << WGM10);
	
	// 비교 일치 A는 비반전 모드, 비교 일치 B는 반전 모드로 설정
	TCCR1A |= (1 << COM1A1);
	TCCR1A |= (1 << COM1B1) | (1 << COM1B0);
	
	// 분주비 256으로 설정
	TCCR1B |= (1 << CS12);
#else	
	// 파형 출력 핀 PB4핀을 출력으로 설정
	DDRB |= (1 << PORTB4);
	
	// 타이머/카운터 0번을 고속 PWM 모드로 설정
	TCCR0 |= (1 << WGM01) | (1 << WGM00);
	
	// 비반전 모드
	TCCR0 |= (1 << COM01);
	
	// 분주비 1,024 설정
	TCCR0 |= (1 << CS02) | (1 << CS01) | (1 << CS00);
#endif
    while (1) 
    {
		OCR0 = dim;
		_delay_ms(10);
		
		dim += direction;
		
		if(dim == 0) direction = 1;
		if(dim == 255) direction = -1;
	
    }
	return 0;
}

// ---- LED 초기화 함수 ----
void init_led()
{
	DDRA=0xff;  // PORTA 를 출력 모드로 설정
	PORTA=0x00;  // PORTA에 물려있는 led를 all off
}
