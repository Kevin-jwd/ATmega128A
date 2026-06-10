/*
 * button.c
 *
 * Created: 2026-06-10 오후 1:25:17
 *  Author: kccistc
 */ 

#include "button.h"

void init_button();
int get_button(int button_number, int button_pin);

// 버튼 초기화 방향설정 (입력)
void init_button()
{
	BUTTON_DDR &= ~(1 << BUTTON0PIN | 1 << BUTTON1PIN | 1 << BUTTON2PIN | 1 << BUTTON3PIN);
	// BUTTON_DDR &= 0xf0;
}

// ---- 버튼 눌림 감지 함수 ----
/* ex. BUTTON0 3
            push : return 1
   release(idle) : return 0 */
int get_button(int button_number, int button_pin)
{
	// 지역변수에 static을 선언하면 함수를 빠져나와도 이전 값을 유지
	// 함수 관리 및 함수/변수 통합 차원에서 전역변수보다 static 변수를 더 선호
	static unsigned char button_status[BUTTON_NUMBER] = 
	{
		BUTTON_RELEASE,
		BUTTON_RELEASE,
		BUTTON_RELEASE,
		BUTTON_RELEASE,
	};
	
	// 1. button값을 읽는다.
	int current_state = BUTTON_PIN & (1 << button_pin);

	// 2. button 상태 확인
	// 버튼이 처음 눌려진 상태
	if(current_state && button_status[button_number] == BUTTON_RELEASE) 
	{
		_delay_ms(15);     // 노이즈 기다림
		button_status[button_number] = BUTTON_PRESS;     // 1
		return 0;          // 아직은 완전히 눌렀다 뗀 상태가 아님
	}
	// 버튼이 이전에 눌려진 상태였으며 지금은 떼어진 상태
	else if(button_status[button_number] == BUTTON_PRESS &&
	current_state == BUTTON_RELEASE) 
	{
		button_status[button_number] = BUTTON_RELEASE;     // 다음 버튼 체크를 위한 초기화
		_delay_ms(10);             
		return 1;                                          // 완전히 1번 눌렀다 뗀 상태로 인정
	}
	return 0;
}