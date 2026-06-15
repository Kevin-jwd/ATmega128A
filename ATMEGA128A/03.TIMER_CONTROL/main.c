/*
 * 03.TIMER_CONTROL.c
 *
 * Created: 2026-06-15 오전 11:36:35
 * Author : kccistc
 */ 
#define F_CPU 16000000UL

#include <avr/io.h>
#include <avr/interrupt.h>         // sei(), cli()

extern int led_main(void);

void init_timer0(void);

volatile uint32_t ms_count = 0;    // volatile 최적화 방지

/*
*             ISR : Interrupt Service Routine
* TIMER0_OVF_vect : Timer/Counter 0 Overflow
* 250개의 펄스를 count(1ms) 하면 이곳으로 자동 진입
*/
ISR(TIMER0_OVF_vect)            // 벡터 번호 17
{
	TCNT0 = 6;   
	ms_count++;                 // 1ms count
}

int main(void)
{
	init_timer0();   
	
	led_main();
	
    while (1) 
    {
		
    }
}

/*
* 1. timer 0 초기화
*    AVR에서 8bit timer 0/2 중 0번을 초기화
*	 임베디드쪽에서 가장 신경을 써야 할 부분이 초기화 하는 것
*	 초기화가 잘못되면 이후에 오류 발생
*
* 2. 8bit로 1ms 측정하는 timer/counter를 생성
*    2.1 분주비 설정 (64분주)
*        16,000,000Hz / 64 = 250,000Hz
*    2.2 1주기 시간 계산
*        T = 1/f = 1/250,000 = 0.000004sec ( 4us, 0.004ms )
*	 2.3 8bit로 count할 시간 계산 (8bit timer overflow)
*        0.000004 x 256 = 0.001024 ( 1.024ms )
*        0.000004 x 250 = 0.001 ( 1ms )
*/
void init_timer0()
{
	TCNT0 = 6;                                        // TCNT0 6~256 
	TCCR0 &= ~(1 << CS02 | 1 << CS01 | 1 << CS00);    // RESET
	TCCR0 |= 1 << CS02 | 0 << CS01 | 0 << CS00;       // SET -- 64분주 설정을 위한 세팅
	TIMSK |= 1 << TOIE0;                              // TIMER0 Overflow Interrupt => 256이 되는 순간 오버플로우에 따른 인터럽트 발생
	sei();                                            // SREG 7번 비트 SET => 전역적으로 인터럽트 허용
	
}