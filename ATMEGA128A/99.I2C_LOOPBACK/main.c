/*
 * 99.I2C_LOOPBACK.c
 *
 * Created: 2026-06-28 오후 3:52:41
 * Author : user
 */ 
#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>    // sei 등 함수 
#include <stdio.h>

extern void init_uart0(void);
extern void UART0_transmit(uint8_t data);
extern int i2c_master_main(void); 

FILE OUTPUT = FDEV_SETUP_STREAM(UART0_transmit, NULL, _FDEV_SETUP_WRITE);

volatile uint32_t ms_counter=0;  // volatile 최적화 방지 

/*
ISR (Interrupt Service Routine) : 인터럽트 처리 함수 ISR로 시작
TIMER0_OVF_vect : Timer 0 Overflow INT가 발생이 되면 이곳으로 진입 함
250개의 펄스를 count(1ms) 하면 이곳으로 자동 진입 한다. 
ISR은 가능한 짧게 작성 한다. 
*/
ISR(TIMER0_OVF_vect)
{
	
	TCNT0 = 6;   // TCNT0 6~256 : 250개 펄스 count하기 위해 
	
	ms_counter++;
}

int main(void)
{
   uint8_t key_value;
   
   init_timer0();
   init_uart0();
   
   stdout = &OUTPUT;   // printf가 동작 할 수 있도록 stdout을 설정 
   sei();   // 전역(대문) interrupt 허용 
   
i2c_master_main();
    while (1) 
    {
		
    }
}

/*
1. timer 0을 초기화 한다.
   AVR에서 8bit timer 0/2 중에서 0번을 초기화 한다. 
   임베디드에서 가장 신경을 써야 할 부분이 초기화 하는것이다. 
   초기화가 잘못 되면 이후가 다 꼬인다. 
 2. 8bit가지고 1ms를 측정 하는 timer/counter를 만들고자 한다. 
  2.1 분주비 설정 (64분주)
      16000000Hz / 64 ==> 250,000Hz
  2.2 1주기가 잡아 먹는시간 계산
      T = 1/f = 1/250,000 ==> 0.000004sec (4us) ==> 0.004ms
  2.3 8bit 가지고 count 하는 시간을 계산 (8bit timer OV(OVflow))
      0.004ms x 256개 ==>  0.001024 sec (1.024ms)
	  0.004ms x 250개 ==>  0.001sec(1ms)
*/

void init_timer0(void)
{
	TCNT0 = 6;   // TCNT0 6~256 : 250개 펄스 count하기 위해 
	
	TCCR0 &= ~(1 << CS02 | 1 << CS01 | 1 << CS00);   // 
	TCCR0 |= 1 << CS02 | 0 << CS01 | 0 << CS00;   // 64분주  
	TIMSK |= 1 << TOIE0;     // TIMER0 Overflow INT 
	
}


