/*
 * 03.TIMER_FND_BTN_CONTROL.c
 *
 * Created: 2026-06-15 오후 4:49:50
 * Author : kccistc
 */ 
#define F_CPU 16000000UL

#include <avr/io.h>
#include <avr/interrupt.h>

#include "button.h"

extern void init_fnd(void);
extern void turn_off_fnd(void);
extern void min_sec_clock(void);
extern void toggle_dot(void);
extern void sec_circular_clock(void);
extern void run_watch(void);
extern void default_stop_watch(void);
extern void reset_watch(void);
extern void display_stopwatch(void);

extern void init_button(void);
extern int get_button(int button_num, int button_pin);

extern uint8_t numbers[10];
extern uint8_t first_circle[10];
extern uint8_t second_circle[10];

volatile uint32_t ms_count = 0;
extern uint32_t sec_count_ms;
extern uint32_t sec_count;
extern uint32_t min_count;

extern uint32_t stopwatch_ms_count;
extern uint32_t stopwatch_sec_count;

extern uint8_t dot_display;

extern int is_stopwatch_on;

void init_timer2(void);

ISR(TIMER2_OVF_vect)            
{
	TCNT2 = 193;
	ms_count++;                 // 1ms count
}

typedef enum STATE { MIN_SEC_CLOCK, SEC_CIRCULAR_CLOCK, 
	DEFAULT_STOP_WATCH, RUN_WATCH, RESET_WATCH };

int main(void)
{
	int current_state = MIN_SEC_CLOCK;
	uint32_t prev_ms = 0;
	
	init_timer2();
	init_button();
	init_fnd();
	
	while (1)
	{
		if(ms_count != prev_ms)
		{
			prev_ms = ms_count;
			
			sec_count_ms++;
			if(sec_count_ms >= 1000)
			{
				sec_count_ms = 0;
				sec_count++;
				toggle_dot();
			}
			if(sec_count >= 60)
			{
				sec_count = 0;
				min_count++;
			}
			
			if(is_stopwatch_on == 1)
			{
				stopwatch_ms_count++;
				if(stopwatch_ms_count >= 1000)
				{
					stopwatch_ms_count = 0;
					stopwatch_sec_count++;
				}
			}
			
			switch(current_state)
			{
				case MIN_SEC_CLOCK:
				min_sec_clock();
				if(get_button(BUTTON0, BUTTON0PIN))
				current_state = SEC_CIRCULAR_CLOCK;
				break;
				case SEC_CIRCULAR_CLOCK:
				sec_circular_clock();
				if(get_button(BUTTON0, BUTTON0PIN))
				current_state = DEFAULT_STOP_WATCH;
				break;
				case DEFAULT_STOP_WATCH:
				default_stop_watch();
				if(get_button(BUTTON0, BUTTON0PIN))
				current_state = MIN_SEC_CLOCK;
				else if(get_button(BUTTON1, BUTTON1PIN))
				current_state = RUN_WATCH;
				else if(get_button(BUTTON2, BUTTON2PIN))
				current_state = RESET_WATCH;
				break;
				case RUN_WATCH:
				run_watch();
				if(get_button(BUTTON1, BUTTON1PIN))
				current_state = DEFAULT_STOP_WATCH;
				break;
				case RESET_WATCH:
				reset_watch();
				if(get_button(BUTTON2, BUTTON2PIN))
				current_state = RUN_WATCH;
				break;
			}
		}
	}
	return 0;
}

/*
* 1. timer 2 초기화
* 2. 8bit로 1ms 측정하는 timer/counter를 생성
*    2.1 분주비 설정 (256분주)
*        16,000,000Hz / 256 = 62,500Hz
*    2.2 1주기 시간 계산
*        T = 1/f = 1/62,500 = 0.000016sec ( 0.016ms )
*	 2.3 8bit로 count할 시간 계산 (8bit timer overflow)
*        0.000016 x 64 = 0.001024 ( 1.024ms )
*        0.000016 x 63 = 0.001008 ( 1.008ms )
*/
void init_timer2()
{
	TCNT2 = 193;                                        // TCNT193 ~256
	TCCR2 &= ~(1 << CS22 | 1 << CS21 | 1 << CS20);    // RESET
	TCCR2 |= 1 << CS22 | 0 << CS21 | 0 << CS20;       // SET -- 64분주 설정을 위한 세팅
	TIMSK |= 1 << TOIE2;                              // TIMER0 Overflow Interrupt => 256이 되는 순간 오버플로우에 따른 인터럽트 발생
	sei();                                            // SREG 7번 비트 SET => 전역적으로 인터럽트 허용
}
