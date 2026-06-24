/*
 * uart0.c
 *
 * Created: 2026-06-16 오전 9:57:57
 *  Author: kccistc
 */ 
#include "uart0.h"

extern int func_state;
extern void (*fp[])();

void init_uart0(void);
void UART0_transmit(uint8_t data);
void process_pc_command(void);

// PC로부터 1byte가 들어오면 인터럽트 서비스 루틴 실행
// ex. 입력이 led_all_on\n 면 11번 진입
ISR(USART0_RX_vect)       // USART0 Rx Complete 벡터 번호 19
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
* 1. 전송 속도 초기화 : 9600bps 
* 2. start/stop bit 설정
* 3. UART Rx : interrupt로 설정
*/
void init_uart0()
{
	// 1. 전송 속도 : 9600bps
	// UBRRnH, UBRRnL (USART Baud Rate Register)
	// H : 상위 4비트, L : 하위 8비트 => 상위 4비트는 사용되지 않음
	UBRR0H = 0x00;
	UBRR0L = 207;       
	
	// UCSRnA (USART Control and Status Register n A)
	// UART 시리얼 통신의 상태를 확인하고 통신 과정을 제어
	UCSR0A |= 1 << U2X0;      // 비트값; 1배속 : 0, 2배속 : 1
	// UCSR0A |= _BV(U2X0); // avr/io.h
	
	// UCSRnB (USART Control and Status Register n B)
	// 데이터 송수신을 가능하도록 설정
	// RXENn (RX Enable) : UART 수신기의 수신 기능 활성화
	// TXENn (TX Enable) : UART 송신기의 송신 기능 활성화
	// RXCIEn (RX Complete Interrupt Enable) : 수신 완료 인터럽트 발생을 허용
	UCSR0B |= 1 << RXEN0 | 1 << TXEN0 | 1 <<  RXCIE0;
}

// ---- UART0로 1byte를 전송하는 함수 ----
// UDREn (USART Data Register Empty) : 송신 버퍼가 비어있을 때 1
void UART0_transmit(uint8_t data)
{
	while( !(UCSR0A & (1 << UDRE0)))         // data가 송신 중이면 송신 끝날 때까지 기다림
	;                                        // no operation
	
	// USART Data Register n
	UDR0 = data;
}

// 
void process_pc_command()
{
	// 데이터가 존재
	if(front != rear)
	{
		printf("%s\n", rx_buff[front]);
		if(strncmp(rx_buff[front], "led_shift_left_on", strlen("led_shift_left_on")) == 0)
		{
			func_state = 0;
		}
		else if(strncmp(rx_buff[front], "led_shift_right_on", strlen("led_shift_right_on")) == 0)
		{
			func_state = 1;
		}
		else if(strncmp(rx_buff[front], "led_shift_left_keep_on", strlen("led_shift_left_keep_on")) == 0)
		{
			func_state = 2;
		}
		else if(strncmp(rx_buff[front], "led_shift_right_keep_on", strlen("led_shift_right_keep_on")) == 0)
		{
			func_state = 3;
		}
		else if(strncmp(rx_buff[front], "led_flower_on", strlen("led_flower_on")) == 0)
		{
			func_state = 4;
		}
		else if(strncmp(rx_buff[front], "led_flower_off", strlen("led_flower_off")) == 0)
		{
			func_state = 5;
		}
		front = (front + 1) % QUEUE_SIZE;
	}
	fp[func_state] ();
}