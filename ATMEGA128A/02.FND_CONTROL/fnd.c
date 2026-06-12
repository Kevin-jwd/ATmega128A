/*
 * fnd.c
 *
 * Created: 2026-06-12 오전 10:52:21
 *  Author: kccistc
 */ 
#include "fnd.h"

void init_fnd(void);
void turn_off_fnd(void);
void display_fnd();

uint32_t ms_count = 0;
uint32_t sec_count = 0;
uint8_t dot_display = 0;

int fnd_main(void)
{
	init_fnd();
	
	while(1)
	{
		display_fnd();
		_delay_ms(1);
		ms_count++;
		if(ms_count >= 1000)
		{
			ms_count = 0;
			sec_count++;
			dot_display = !dot_display;
		}
	}
	return 0;
}

// ---- FND 초기화 함수 ----
void init_fnd(void)
{
	FND_DATA_DDR = 0xFF;          // 출력 모드로 설정
	FND_CONTROL_DDR |= 1 << FND_CONTROL_D1 | 1 << FND_CONTROL_D2 | 1 << FND_CONTROL_D3 | 1 << FND_CONTROL_D4;       
	turn_off_fnd();
}

// ---- 전부 끄기 함수 ----
// common cathode
void turn_off_fnd(void)
{
	FND_DATA_PORT = 0x00;       // 전위 차를 맞추기 위한 설정
	// FND_DATA_PORT = ~0xFF;
}

void display_fnd()
{
	uint8_t numbers[10] = { 0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x27, 0x7F, 0x67};
	static int digit_select = 0;
	
	switch(digit_select)
	{
		case 0:     // 1단위
#if 0
			FND_CONTROL_PORT = 0x80;
#else
			FND_CONTROL_PORT = ~0x80;
#endif
			FND_DATA_PORT = numbers[sec_count % 10];
			break;
			
		case 1:     // 10단위
#if 0
			FND_CONTROL_PORT = 0x40;
#else
			FND_CONTROL_PORT = ~0x40;
#endif
			FND_DATA_PORT = numbers[sec_count/10 % 6];
			break;
			
		case 2:     // 100단위
#if 0
			FND_CONTROL_PORT = 0x20;
#else
			FND_CONTROL_PORT = ~0x20;
#endif
			FND_DATA_PORT = numbers[sec_count/60 % 10];
			break;
			
		case 3:     // 1000단위
#if 0
			FND_CONTROL_PORT = 0x10;
#else
			FND_CONTROL_PORT = ~0x10;
#endif
			FND_DATA_PORT = numbers[sec_count/600 % 6];
			break;		
	}
	digit_select = (digit_select + 1) % 4;   // 다음 표시할 자리수
	
}