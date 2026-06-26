/*
 * DHT11.c
 *
 * Created: 2026-06-26 오전 9:27:55
 *  Author: kccistc
 */ 
#include "DHT11.h"
#define DHT11_DDR DDRG          // DATA PORT : PORT G
#define DHT11_PORT PORTG
#define DHT11_PIN PING       
#define DHT11_INPUT_PIN 0       // DATA PIN : 0
#define TIMEOUT_MAXIMUM_US 100
#define DATA_BYTE_SIZE 8
#define DATA_SIZE 5

void dht11_main(void);

uint8_t checksum_byte = 0x00;

enum t_state {
	OK,
	STARTUP_TIMEOUT_ERROR,      
	RESPONSE_TIMEOUT_ERROR,   
	DATA_TIMEOUT_ERROR,
	CHECKSUM_ERROR,
	};

// Single Data Bus 이기 떄문에 Host에서 보낼 때는 DDR 출력모드, Slave에서 보낼 때는 DDR 입력모드 전환
void dht11_main()
{
	uint8_t  bytes[DATA_SIZE+1];           // 8bit data x 6 (integer RH, decimal RH, integer T, decimal T, checksum, )
	uint8_t  state      = OK;
	int      us_counter = 0;
	
	// 1. Initialize variables
	state = OK;
	memset(bytes, 0, sizeof(bytes));

	/* 2. Startup Signal */
	// 2-1. 
	DHT11_DDR  |= 1 << DHT11_INPUT_PIN;       // DDR 출력 모드 설정
	DHT11_PORT |= 1 << DHT11_INPUT_PIN;       // high   
	_delay_ms(100);                           // high 유지
	
	// 2-2. MCU sends out Start Signal and pulls down voltage for at least 18ms to let DHT11 detect the signal
	DHT11_PORT &= ~(1 << DHT11_INPUT_PIN);    // low
	_delay_ms(20);                            // low 유지 (>18ms)
	
	// 2-3. MCU pulls up voltage and waits for DHT11
	DHT11_PORT |= 1 << DHT11_INPUT_PIN;       // high
	_delay_us(30);                            // high 유지 (20~40us)
	
	/* 3. DHT Responses */
	// DHT11 send out response signal and keeps it for 80us
	DHT11_DDR &= ~(1 << DHT11_INPUT_PIN);     // DDR 입력 모드 설정
	
	// 3-1. Check if signal turns into low between MCU startup signal & DHT11 response signal (maximum: 100us)
	us_counter = 0;
	while(DHT11_PIN & (1 << DHT11_INPUT_PIN))
	{
		_delay_us(1);
		if(++us_counter > TIMEOUT_MAXIMUM_US) { state = STARTUP_TIMEOUT_ERROR; break; }
	}
	// 3-2. Check if DHT11 response with low signal (maximum: 100us)
	us_counter = 0;
	if(state == OK)
	{
		while(!(DHT11_PIN & (1 << DHT11_INPUT_PIN)))
		{
			_delay_us(1);
			if(++us_counter > TIMEOUT_MAXIMUM_US) { state = RESPONSE_TIMEOUT_ERROR; break; }
		}	
	}
	// 3-3. Check if DHT11 response with high signal (maximum: 100us)
	us_counter = 0;
	if(state == OK)
	{
		while(DHT11_PIN & (1 << DHT11_INPUT_PIN))
		{
			_delay_us(1);
			if(++us_counter > TIMEOUT_MAXIMUM_US) { state = RESPONSE_TIMEOUT_ERROR; break; }
		}
	}
	
	/* 4. Data Transmission */
	// 4-1. read 40bits data
	if(state == OK)
	{
		for(int i = 0;i < DATA_SIZE;i++)
		{
			uint8_t one_byte = 0;
			for(int j = 0;j < DATA_BYTE_SIZE;j++)
			{
				// Check if low signal keeps itself for 50us
				us_counter = 0;
				while(!(DHT11_PIN & (1 << DHT11_INPUT_PIN)))
				{
					_delay_us(1);
					if(++us_counter > TIMEOUT_MAXIMUM_US) { state = DATA_TIMEOUT_ERROR; break; }
				}
				// Check length of high signal (0: 26-28us, 1: 70us)
				us_counter = 0;
				while(DHT11_PIN & (1 << DHT11_INPUT_PIN))
				{
					_delay_us(1);
					if(++us_counter > TIMEOUT_MAXIMUM_US) { state = DATA_TIMEOUT_ERROR; break; }
				}
				// >=30us; 1
				if(us_counter >= 30) { one_byte |= 1 << (DATA_BYTE_SIZE-1-j); }
				
			}
			bytes[i] = one_byte;        
		}
	}
	// 4-2. Check checksum bits
	if(state == OK)
	{
		// Calculate checksum byte : checksum byte = sum of other 4 bytes
		checksum_byte = bytes[0] + bytes[1] + bytes[2] + bytes[3];
		if(bytes[DATA_SIZE - 1] != checksum_byte) { state = CHECKSUM_ERROR; }
	}
	switch(state)
	{
		case OK:
			// Received Data: Integer RH, Decimal RH, Integer T, Decimal T
			printf("Humidity: %d.%d, Temperature: %d.%d\n", bytes[0], bytes[1], bytes[2], bytes[3]);
			break;
		case STARTUP_TIMEOUT_ERROR:
			printf("Error : Startup timeout\n");
			break;
		case RESPONSE_TIMEOUT_ERROR:
			printf("Error : Response timeout\n");
			break;			
		case DATA_TIMEOUT_ERROR:
			printf("Error : Data timeout\n");
			break;
		case CHECKSUM_ERROR:
			printf("Error : Incorrect checksum byte\nReceived : %0x, Calculated : %0x", bytes[DATA_SIZE - 1], checksum_byte);
			break;
		default:
			printf("Error : Invalid State\n");
			break;
	}
}