/*
 * 02.FND_CONTROL_MY.c
 *
 * Created: 2026-06-12 오후 1:26:05
 * Author : kccistc
 */ 

#include <avr/io.h>
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

extern uint32_t ms_count;
extern uint32_t sec_count;
extern uint32_t min_count;

extern uint32_t stopwatch_ms_count;
extern uint32_t stopwatch_sec_count;

extern uint8_t dot_display;

extern int is_stopwatch_on;

typedef enum STATE { MIN_SEC_CLOCK, SEC_CIRCULAR_CLOCK, 
	DEFAULT_STOP_WATCH, RUN_WATCH, RESET_WATCH };

int main(void)
{
	int current_state = MIN_SEC_CLOCK;
	
	init_button();
	init_fnd();
	
	while (1)
	{
		// 시계 
		_delay_ms(1);
		ms_count++;
		
		if(is_stopwatch_on == 1)
		{
			stopwatch_ms_count++;
		}
		
		if(ms_count >= 1000)
		{
			ms_count = 0;
			sec_count++;
		}
		
		if(stopwatch_ms_count > 1000)
		{
			stopwatch_ms_count = 0;
			stopwatch_sec_count++;
		}
		
		if(sec_count >= 60)
		{
			sec_count =0 ;
			min_count++;
		}
		
		// 상태 천이
		switch(current_state)
		{
			case MIN_SEC_CLOCK:
				min_sec_clock();
				if(get_button(BUTTON0, BUTTON0PIN))
				{
					current_state = SEC_CIRCULAR_CLOCK;
				}
				break;
			case SEC_CIRCULAR_CLOCK:
				sec_circular_clock();
				if(get_button(BUTTON0, BUTTON0PIN))
				{
					current_state = DEFAULT_STOP_WATCH;
				}
				break;
			case DEFAULT_STOP_WATCH:
				default_stop_watch();
				if(get_button(BUTTON0, BUTTON0PIN))
				{
					current_state = MIN_SEC_CLOCK;
				}
				else if(get_button(BUTTON1, BUTTON1PIN))
				{
					current_state = RUN_WATCH;
				}
				else if(get_button(BUTTON2, BUTTON2PIN))
				{
					current_state = RESET_WATCH;
				}
				break;
			case RUN_WATCH:
				run_watch();
				if(get_button(BUTTON1, BUTTON1PIN))
				{
					current_state = DEFAULT_STOP_WATCH;
				}
				break;
			case RESET_WATCH:
				reset_watch();
				if(get_button(BUTTON2, BUTTON2PIN))
				{
					current_state = RUN_WATCH;
				}
				break;
		}
	}
	return 0;
}

