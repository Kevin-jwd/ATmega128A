/*
 * keypad.c
 *
 * Created: 2026-06-29 오후 2:11:19
 *  Author: kccistc
 */ 
#include "keypad.h"

void init_keypad(void);
uint8_t scan_keypad(void);
uint8_t get_button(int row, int column);

uint8_t operator = '+';

void init_keypad()
{
	KEYPAD_DDR  = 0x0f;            // row: 입력, column: 출력
	KEYPAD_PORT = 0xff;           
}

uint8_t scan_keypad()
{
	uint8_t data = 0;              // key값을 저장하는 변수
	for(int row = 0;row < ROW_LENGTH;row++)
	{
		for(int column = 0;column < COLUMN_LENGTH;column++)
		{
			data = get_button(row, column);
			if(data) return data;
		}
	}
	return 0;
}

uint8_t get_button(int row, int column)
{
	uint8_t keypad_char_arr[ROW_LENGTH][COLUMN_LENGTH] = {
		{'1', '2', '3', '+'},
		{'4', '5', '6', '-'},
		{'7', '8', '9', '*'},
		{'=', '0', '=', '/'}
	};
	
	static int8_t prev_state_arr[ROW_LENGTH][COLUMN_LENGTH] =
	{
		{ 1, 1, 1, 1 },  
		{ 1, 1, 1, 1 },
		{ 1, 1, 1, 1 },
		{ 1, 1, 1, 1 }	                       // 초기 버튼은 눌려지지 않은 상태 (Active-Low 동작)
	};
	
	int8_t current_state = 1;
	
	KEYPAD_PORT = 0xff;
	KEYPAD_PORT &= ~(1 << 3-column);             // 해당 column에 전류 흘림

	for(int delay = 0;delay < 1000;delay++)      // keypad 확인을 위한 delay 0.625us x 20 = 1.25us
		;
		
	current_state = ((KEYPAD_PIN & (1 << (row + 4))) >> (row + 4));
		
	// ex. row 0 : PA4에 연결
	// 76543210
	// 11101111 KEYPAD_PIN & (1 << (row+4))
	// 00001110 >> (row+4)
	if(current_state == 0 && prev_state_arr[row][column] == 1)         // 처음 눌려진 상태
	{
		prev_state_arr[row][column] = 0;                          
		return 0;
	}
	else if(current_state == 1 && prev_state_arr[row][column] == 0)    // 이전에 버튼이 눌려지고 현재 뗀 상태 
	{
		prev_state_arr[row][column] = 1;
		printf("key : %c\n", keypad_char_arr[row][column]);
		return keypad_char_arr[row][column];
	}
	return 0;
}
