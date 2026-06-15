/*
 * fnd.c
 *
 * Created: 2026-06-12 오후 1:33:10
 *  Author: kccistc
 */ 
#include "fnd.h"

void init_fnd(void);
void turn_off_fnd(void);
void min_sec_clock(void);
void toggle_dot(void);
void sec_circular_clock(void);
void run_watch(void);
void default_stop_watch(void);
void reset_watch(void);
void display_stopwatch(void);

// ---- FND 초기화 함수 ----
void init_fnd(void)
{
	FND_DATA_DDR = 0xFF;          // 출력 모드로 설정
	FND_CONTROL_DDR |= 1 << FND_CONTROL_D1 | 1 << FND_CONTROL_D2 | 
	1 << FND_CONTROL_D3 | 1 << FND_CONTROL_D4;       
	turn_off_fnd();
}

// ---- 전부 끄기 함수 ----
// common cathode
void turn_off_fnd(void)
{
	FND_DATA_PORT = 0x00;       // 전위 차를 맞추기 위한 설정
	// FND_DATA_PORT = ~0xFF;
}

// ---- 1) 분.초 시계 (1초 단위 도트 on/off) ----
void min_sec_clock(void)
{
	static int digit_select = 0;
	
	switch(digit_select)
	{
		// sec 1 단위
		case 0:
			FND_CONTROL_PORT = ~0x80;
			FND_DATA_PORT = numbers[sec_count % 10];
			break;
		// sec 10 단위
		case 1:
			FND_CONTROL_PORT = ~0x40;
			FND_DATA_PORT = numbers[sec_count/10 % 6] | dot_display;
			break;
		// min 100 단위
		case 2:
			FND_CONTROL_PORT = ~0x20;
			FND_DATA_PORT = numbers[min_count % 10];
			break;
		// min 1000 단위
		case 3:
			FND_CONTROL_PORT = ~0x10;
			FND_DATA_PORT = numbers[min_count/10 % 6];
			break;
	}
	if(ms_count == 0)
	{
		toggle_dot();
	}

	
	digit_select = (digit_select + 1) % 4;   // 다음 표시할 자리수
}

// ---- 도트 토글 함수 ----
void toggle_dot()
{
	dot_display = (dot_display == 0x80) ? 0x00 : 0x80;
}

// ---- 2) 원형 디스플레이 및 초 단위 시계 출력 ----
void sec_circular_clock(void)
{
	static int digit_select = 0;
	
	switch(digit_select)
	{
		// sec 1 단위
		case 0:
			FND_CONTROL_PORT = ~0x80;
			FND_DATA_PORT = numbers[sec_count % 10];
			break;
		// sec 10 단위
		case 1:
			FND_CONTROL_PORT = ~0x40;
			FND_DATA_PORT = numbers[sec_count/10 % 6];
			break;
		
		// circular display
		case 2:
			FND_CONTROL_PORT = ~0x20;
			FND_DATA_PORT = first_circle[sec_count % 8];
		break;
		case 3:
			FND_CONTROL_PORT = ~0x10;
			FND_DATA_PORT = second_circle[sec_count % 8];
		break;
	}
	digit_select = (digit_select + 1) % 4;
}

// ---- 3-1) (STOP) 스탑 워치 ----
// 기본 동작 (default)
// 10초 | 1초 | 100밀리초 | 10밀리초
void default_stop_watch(void)
{
	is_stopwatch_on = 0;
	
	display_stopwatch();
}

// ---- 3-2) RUN 스탑 워치
void run_watch(void)
{
	is_stopwatch_on = 1;
	
	display_stopwatch();
}

// ---- 3-3) RESET 스탑워치 ----
void reset_watch(void)
{
	stopwatch_ms_count = 0;
	stopwatch_sec_count = 0;
	display_stopwatch();
}

// ---- 스탑워치 기본 출력 포맷 ----
void display_stopwatch()
{
	static int digit_select = 0;
		
	switch(digit_select)
	{
		// ms 10 단위
		case 0:
		FND_CONTROL_PORT = ~0x80;
		FND_DATA_PORT = numbers[stopwatch_ms_count/10 % 10];
		break;
		// ms 100 단위
		case 1:
		FND_CONTROL_PORT = ~0x40;
		FND_DATA_PORT = numbers[stopwatch_ms_count / 100];
		break;
		// sec 1 단위
		case 2:
		FND_CONTROL_PORT = ~0x20;
		FND_DATA_PORT = numbers[stopwatch_sec_count % 10];
		break;
		// sec 10 단위
		case 3:
		FND_CONTROL_PORT = ~0x10;
		FND_DATA_PORT = numbers[stopwatch_sec_count/10 % 6];
		break;
	}
	digit_select = (digit_select + 1) % 4;
}