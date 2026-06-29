/*
 * dht11.c
 *
 * Created: 2026-06-26 오전 9:28:05
 *  Author: user
 */ 

#include "dht11.h"

#define DHT11_DDR  DDRG
#define DHT11_PORT  PORTG
#define DHT11_PIN   PING
#define DHT11_INPUT_PIN 0

void dht11_main(void);

enum t_state { OK, STARTUP_TIMEOUT, DATA_TIMEOUT, CHECKSUM_ERROR}; 
	
void dht11_main(void)
{
	uint8_t bytes[6];
	uint8_t state=0;
	int     us_counter=0; 
	
	// 1. 변수 초기화
	state=OK;
	memset(bytes,0,sizeof(bytes));
	 
	//-----------------------------------------------------------
	//  start up signal scenario 
	//-----------------------------------------------------------
	// 1.  start up signal 전송 
	//--- 1.1 reset DHT11
	DHT11_DDR |= 1 << DHT11_INPUT_PIN;   // output mode
	DHT11_PORT |= 1 << DHT11_INPUT_PIN;  // high
	_delay_ms(100);
	
	//--- 1.2 low 최소 18ms	
	DHT11_PORT &= ~(1 << DHT11_INPUT_PIN);  // low
	_delay_ms(20);   // spec상으로 최소 18ms유지 	

	DHT11_PORT |= 1 << DHT11_INPUT_PIN;  // pull-up
	_delay_us(30);   // spec: 20~40us 유지 

	//2. start signal 응답 check 	
	DHT11_DDR &= ~(1 << DHT11_INPUT_PIN);   // input mode 전환 
	// 	2.1 DHT11이 low로 응답 하는지 check (최대 100us까지 기다려 주겠다)
	us_counter=0;
	while ( (DHT11_PIN & ( 1 << DHT11_INPUT_PIN)))
	{
		_delay_us(1);
		if (++us_counter > 100) { state=STARTUP_TIMEOUT; break; }
	}
	// 2.2 low구간 유지 확인 (80us)
	if (state == OK)
	{
		us_counter=0;
		while ( !(DHT11_PIN & ( 1 << DHT11_INPUT_PIN)) )
		{
			_delay_us(1);
			if (++us_counter > 100) { state=STARTUP_TIMEOUT; break; }
		}	
	}
	
	// 2.3 high 구간 유지 확인 (80us)
	if (state == OK) // 이후는 start data transmission 
	{
		us_counter=0;
		while ( (DHT11_PIN & ( 1 << DHT11_INPUT_PIN)) )
		{
			_delay_us(1);
			if (++us_counter > 100) { state=STARTUP_TIMEOUT; break; }
		}
	}
	
	//---------------------------------------------------------------
	//----------- DATA read Part ------------------------------------
	//---------------------------------------------------------------
	// 3.1 data 40bits 읽는 작업
	if (state == OK)
	{
		for (int i=0; i < 5; i++)
		{
			uint8_t one_byte=0;
			for (int j=0; j < 8; j++)
			{
				// 1. low 구간 50us check 
				us_counter=0;
				while ( !(DHT11_PIN & ( 1 << DHT11_INPUT_PIN)) )
				{
					_delay_us(1);
					if (++us_counter > 100) { state=DATA_TIMEOUT; break; }
				}
				// 2. high 구간 길이 check (spec 0:26~28us 1: 70us)
				//     30us 이내면 0 이상이면 1 로 판단하도록 로직을 구성 한다. 
				us_counter=0;
				while ( (DHT11_PIN & ( 1 << DHT11_INPUT_PIN)) )
				{
					_delay_us(1);
					if (++us_counter > 100) { state=DATA_TIMEOUT; break; }
				}
				//     30us 이내면 0 이상이면 1 로 판단하겠다
				if (us_counter > 30)
					one_byte |= 1 << (7-j); 
			}
			bytes[i] = one_byte;   
		}
	}
	
	// 3.2 checksum 확인 
	uint8_t checksum;
	if (state == OK)
	{
		checksum = bytes[0] + bytes[1] + bytes[2] + bytes[3];
		if (bytes[4] != checksum)
			state = CHECKSUM_ERROR;
	}
	
	switch(state) {
		case OK:
			printf("humi: %d.%d\n", bytes[0], bytes[1]);
			printf("temp: %d.%d\n", bytes[2], bytes[3]);
			break;
		case STARTUP_TIMEOUT:
			printf("STARTUP_TIMEOUT !!!!\n");
			break;
		case DATA_TIMEOUT:
			printf("DATA_TIMEOUT !!!!\n");
			break;
		case CHECKSUM_ERROR:
			printf("CHECKSUM_ERROR bytes[4]:%0x checksum:%0x !!!!\n", bytes[4], checksum);
			break;
	}
}