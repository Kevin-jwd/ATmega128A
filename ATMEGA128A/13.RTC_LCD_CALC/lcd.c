/*
 * text_lcd.c
 *
 * Created: 2026-07-01 9:59:21
 *  Author: kccistc
 */ 

#include "lcd.h"

extern uint8_t timekeeper_register_data[7];

void init_lcd(void);
int lcd_main(void);
void lcd_timer(void);
void lcd_write_dec_chars(uint8_t dec_data);

void init_lcd(void)
{
	_delay_ms(50);		//초기 구동 시간
	
	LCD_DATA_DDR |= 0xf0;	//PB4-7 출력으로 설정
	LCD_DATA_PORT = 0x00;	//data port low
	LCD_CONTROL_DDR |= 1 << RS_PIN | 1 << RW_PIN | 1 << E_PIN;	//제어핀 출력 설정
	LCD_CONTROL_PORT &= ~(1 << RW_PIN);		//r/w핀에 low 출력, 쓰기 전용으로 사용(r: 1, w: 0)
	
	lcd_write_command(0x02);	
	lcd_write_command(COMMAND_4_BIT_MODE);

	uint8_t command = 0x08 | (1 << COMMAND_DISPLAY_ON_OFF_BIT);		
	lcd_write_command(command);		//display on, cursor off, blink off
	
	lcd_clear();				//화면 지우기
	
	lcd_write_command(0x06);	//Entry mode set: 출력 후 커서를 오른쪽으로 옮김. DDRAM의 주소 증가, 화면 이동 X
}

void lcd_pulse_enable(void)
{
	LCD_CONTROL_PORT |= (1 << E_PIN);	//E_pin: HIGH
	_delay_us(1);
	LCD_CONTROL_PORT &= ~(1 << E_PIN);	//E_pin: LOW, 하강 엣지에서 데이터 처리
	_delay_us(1);
}

void lcd_clear(void)
{
	lcd_write_command(COMMAND_CLEAR_DISPLAY);
	_delay_ms(2);
	printf("clear\n");
}

void lcd_write_command(uint8_t command)
{
	LCD_CONTROL_PORT &= ~(1 << RS_PIN);			//RS핀 0으로 설정, 0: 명령어, 1:데이터
	
	LCD_DATA_PORT = command & 0xf0;;		//상위니블 처리
	lcd_pulse_enable();
	LCD_DATA_PORT = (command << 4) & 0xf0;
	lcd_pulse_enable();
	
	_delay_ms(2);
}

void lcd_write_data(uint8_t data)
{
	LCD_CONTROL_PORT |= 1 << RS_PIN;	//RS핀 1로 설정
	
	LCD_DATA_PORT = data & 0xf0;		//상위니블 처리
	lcd_pulse_enable();
	LCD_DATA_PORT = (data << 4) & 0xf0;		//하위니블 처리
	lcd_pulse_enable();
	
	_delay_ms(2);
}

void lcd_write_string(char *string)
{
	for(uint8_t i = 0; string[i]; i++)
	{
		lcd_write_data(string[i]);
	}
}

void lcd_write_dec_chars(uint8_t dec_data)
{
	uint8_t ten_char = (dec_data / 10) + '0'; 
	uint8_t one_char = (dec_data % 10) + '0';  

	lcd_write_data(ten_char);
	lcd_write_data(one_char);
}

void lcd_goto_xy(uint8_t row, uint8_t col)
{
	col %= 16;	//0~15
	row %= 2;	//0~1
	
	//row1 시작 주소 0x00, row2 시작 주소 0x40
	uint8_t address = (0x40 * row) + col;
	uint8_t command = 0x80 + address;
	
	lcd_write_command(command);		//커서 이동
}

int lcd_main(void)
{
	init_lcd();
	
	lcd_write_string("hello");
	
 	_delay_ms(1000);
	
	lcd_clear();
	
	lcd_goto_xy(0, 0);
	lcd_write_data('1');
	lcd_goto_xy(0, 5);
	lcd_write_data('2');
	lcd_goto_xy(0, 10);
	lcd_write_data('3');
	lcd_goto_xy(1, 0);
	lcd_write_data('4');

	while(1);
	return 0;
}

void lcd_timer(void)
{
	lcd_goto_xy(0, 0);
	
	lcd_write_data('2');
	lcd_write_data('0');
	
	lcd_write_dec_chars(timekeeper_register_data[0]);   // year
	lcd_write_data('-');
	
	lcd_write_dec_chars(timekeeper_register_data[1]);   // month
	lcd_write_data('-');
	
	lcd_write_dec_chars(timekeeper_register_data[2]);   // date
	
	lcd_goto_xy(1,0);
	
	lcd_write_dec_chars(timekeeper_register_data[4]);   // hour
	lcd_write_data(':');
	
	lcd_write_dec_chars(timekeeper_register_data[5]);   // minute
	lcd_write_data(':');
	
	lcd_write_dec_chars(timekeeper_register_data[6]);   // second
}