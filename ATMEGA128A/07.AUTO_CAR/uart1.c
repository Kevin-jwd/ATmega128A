/*
 * uart0.c
 *
 * Created: 2026-06-16 오전 9:57:57
 *  Author: kccistc
 */ 
#include "uart1.h"

extern void UART0_transmit(uint8_t data);

extern int func_state;
extern void (*fp[])();

void init_uart1(void);
void UART1_transmit(uint8_t data);
extern void UART0_transmit(uint8_t data);         
 
volatile uint8_t bt_data;

// PC로부터 1byte가 들어오면 인터럽트 서비스 루틴 실행
// ex. 입력이 led_all_on\n 면 11번 진입

ISR(USART1_RX_vect)       // USART1 Rx Complete 
{	
	bt_data = UDR1;                   // UDR1의 내용이 data에 복사된 후 UDR1는 비움
	UART0_transmit(bt_data);          // BT로부터 들어온 byte를 확인하기 위해서 comport master로 출력
}

/*
* 1. 전송 속도 초기화 : 9600bps 
* 2. start/stop bit 설정
* 3. UART Rx : interrupt로 설정
*/
void init_uart1()
{
	// 1. 전송 속도 : 9600bps
	// UBRRnH, UBRRnL (USART Baud Rate Register)
	// H : 상위 4비트, L : 하위 8비트 => 상위 4비트는 사용되지 않음
	UBRR1H = 0x00;
	UBRR1L = 207;       
	
	// UCSRnA (USART Control and Status Register n A)
	// UART 시리얼 통신의 상태를 확인하고 통신 과정을 제어
	UCSR1A |= 1 << U2X1;      // 비트값; 1배속 : 0, 2배속 : 1
	// UCSR0A |= _BV(U2X0); // avr/io.h
	
	// UCSRnB (USART Control and Status Register n B)
	// 데이터 송수신을 가능하도록 설정
	// RXENn (RX Enable) : UART 수신기의 수신 기능 활성화
	// TXENn (TX Enable) : UART 송신기의 송신 기능 활성화
	// RXCIEn (RX Complete Interrupt Enable) : 수신 완료 인터럽트 발생을 허용
	UCSR1B |= 1 << RXEN1 | 1 << TXEN1 | 1 <<  RXCIE1;
}

// ---- UART0로 1byte를 전송하는 함수 ----
// UDREn (USART Data Register Empty) : 송신 버퍼가 비어있을 때 1
void UART1_transmit(uint8_t data)
{
	while( !(UCSR1A & (1 << UDRE1)))         // data가 송신 중이면 송신 끝날 때까지 기다림
	;                                        // no operation
	
	// USART Data Register n
	UDR1 = data;
}