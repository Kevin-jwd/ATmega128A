/*
 * uart0.c
 *
 * Created: 2026-06-16 오전 9:57:59
 *  Author: user
 */ 
#include "uart0.h"
#include "ds1302.h"

extern int func_state;
extern void (*fp[]) ();

void init_uart0(void);
void UART0_transmit(uint8_t data);
void pc_command_processing(t_ds1302 *sp);

// P278 표12-3
// PC로부터 1byte가 들어 오면 자동적으로 이곳으로 진입 한다. 
// 예) led_all_on\n 이면 11번 이곳으로 진입 한다. 
ISR(USART0_RX_vect)
{
	volatile uint8_t data;
	volatile static int i = 0;
		
	data = UDR0;          // UDR0의 내용이 data에 복사된 후 UDR0는 비움
		
	// is full
	if((rear+1) % QUEUE_SIZE == front)
	{
		return;
	}
		
	// 다음 데이터
	if(data == '\n' || data == '\r')
	{
		// is not full
		rx_buff[rear][i] = NULL;         // 문장의 끝 삽입
		i = 0;                           // 다음 string 저장 위해서 초기화
		rear = (rear + 1) % QUEUE_SIZE;  // rear : 0~9
	}
	// 데이터 이어받기
	else
	{
		rx_buff[rear][i++] = data;
	}
}
/*
1. 전송속도 : 9600bps 
*/
void init_uart0(void)
{
	// 1. 전송속도 : 9600bps 
	UBRR0H = 0x00;
	UBRR0L = 207;   //9600BPS 표8-9
	UCSR0A |= 1 << U2X0;  // 2배속 설정 (sampling 8)
	// UART0 를 송신.수신이 다 가능하고 RX INT가 가능 하도록 설정 한다.
	UCSR0B |= 1 << RXEN0 | 1 << TXEN0 | 1 << RXCIE0;   
}

// UART0 로 1byte를 전송 하는 함수 
void UART0_transmit(uint8_t data)
{
	while ( !(UCSR0A & 1 << UDRE0))   // data가 송신중이면 송신이 끝날때 까지 기다림
		;     // No operation
	UDR0 = data;   // HW 전송 register에 data를 송신 한다. 
}


void pc_command_processing(t_ds1302 *sp)
//   setrtc260629095600
{
	if (front != rear)    // data가 rx_buff에 존재 하는지 check   
	{
		if (strncmp((char *) rx_buff[front], "setrtc", strlen("setrtc")) == 0)
		{
			int year, month, date, hour, minute, second;
			int parsed_command = sscanf(&rx_buff[front][6], "%2d%2d%2d%2d%2d%2d", &year, &month, &date, &hour, &minute, &second);
			
			write_ds1302(ADDR_YEAR, year);
			write_ds1302(ADDR_MONTH, month);
			write_ds1302(ADDR_DATE, date);
			write_ds1302(ADDR_HOUR, hour);
			write_ds1302(ADDR_MINUTES, minute);
			write_ds1302(ADDR_SECONDS, second);
		} 
		front = (front + 1) % QUEUE_SIZE;
	}
}