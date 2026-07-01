/*
 * i2c_s_loopback.c
 *
 * Created: 2026-06-30 오후 2:28:11
 *  Author: kccistc
*/

#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <string.h>

/* I2C settings */
#define SLAVE_ADDRESS     0x50  // 7bit slave address

//------------- TWI Status code (Slave) -------------
// TWSR (TWI Status register) : bit 7~3 (5) ==> I2C통신의 전송 상태나 오류를 나타냄
// 아래의 코드를 수신하면 ISR(TWI_vect) 상태로 분기됨

/* Slave Receiver */
#define TWSR_SR_SLA_ACK     0x60    // Slave가 SLA + W 수신 후 ACK 송신
#define TWSR_SR_DATA_ACK    0x80    // Slave가 Data 수신 후 ACK 송신
#define TWSR_SR_STOP        0xA0    // STOP 감지

#define TWSR_ST_SLA_ACK     0xA8     // Slave가 SLA + R 수신 후 ACK 송신
#define TWSR_ST_DATA_ACK    0xB8     // Slave가 Data 송신 후 ACK 수신
#define TWSR_ST_DATA_NACK   0xC0     // Slave가 Data 송신 후 NACK 수신
#define TWSR_ST_LAST_DATA   0xC8     // 마지막 데이터 송신 + ACK 수신

#define RX_SIZE 8

/*
*  ATmega128a TWI (I2C) 관련 register
*  TWCR : TWI Control Register
*     - 각 bit의 역할
*     - TWINT : 인터럽트 플래그 (1: clear(일반 flag와 다름)) I2C작업이 끝나면 set
*     - TWEA : TWI Enable ACK 
*     - TWSTA : START 신호 발생 (Master 전용)
*     - TWSTO : STOP 신호 발생 (Master 전용)
      - TWEN  : TWI(I2C) Enable (I2C HW 활성화)
	  - TWIE  : Interrupt Enable (I2C interrupt를 활성화)
*  TWSR : TWI Status Register  (상위 5bit = 상태 코드, 하위 2bit = Prescaler)
*  TWBR : TWI Bit Register 
*  TWDR : TWI Data Register
*  TWAR : TWI Address Register (Slave 모드에서 사용, 자신의 Address를 설정)
*/

void i2c_slave_main(void);
void init_slave_i2c(void);

volatile uint8_t rx_buff[RX_SIZE];
volatile int rx_len=0;             // 수신된 byte 수
volatile int tx_idx=0;             // 송신 index

/* TWI interrupt service routine */
// Slave 상태 시 인터럽트 발생
ISR(TWI_vect)
{
	uint8_t st = TWSR & 0xf8;            // 7~3 bit 값 취함
	
	switch(st)
	{    
		
		// =========== Slave 수신 단계 ===========
		
		// SLA + W 수신 => ACK 송신
		case TWSR_SR_SLA_ACK:
			rx_len = 0;                                                // 버퍼 초기화
			TWCR = 1 << TWINT | 1 << TWEA | 1 << TWEN | 1 << TWIE;     // ACK가 자동 송신됨
			break;
			
		// DATA 수신 => ACK 송신 (반복)
		case TWSR_SR_DATA_ACK:
			// 데이터 1byte 수신
			if(rx_len < RX_SIZE) 
			{ 
				rx_buff[rx_len++] = TWDR;
			}
			TWCR = 1 << TWINT | 1 << TWEA | 1 << TWEN | 1 << TWIE;     // ACK가 자동 송신됨
			break;
			
		// STOP 감지
		case TWSR_SR_STOP:       
			tx_idx = 0;                                                // Read 요청에 처음부터 돌려줄 수 있도록 설정
			TWCR = 1 << TWINT | 1 << TWEA | 1 << TWEN | 1 << TWIE;     // 읽기 모드 대기
			break;  			
			 
		// =========== Slave 송신 단계 ===========		
			
		// SLA + R 수신 => ACK 송신  
		case TWSR_ST_SLA_ACK:	   
			tx_idx = 0;        // 첫 번째 byte 수신 준비
			// break; 바로 넘어감
			
		// ACK 수신 => 다음 byte 전송
		case TWSR_ST_DATA_ACK:	
			if(tx_idx < rx_len)	  
			{
				TWDR = rx_buff[tx_idx++];
			}
			else
			{
				TWDR = 0xff;           // 버퍼 이상 발생; dummy byte
			}
			// 마지막 바이트라면  TWEA = 0으로 NACK 송신 -> 더 이상 송신할 데이터가 없음
			if(tx_idx >= rx_len)
			{
				TWCR = 1 << TWINT | 1 << TWEN | 1 << TWIE;               // TWEA = 0; NACK 준비
			}
			else
			{
				TWCR = 1 << TWINT | 1 << TWEA | 1 << TWEN | 1 << TWIE;    // TWEA = 1; ACK 준비
			}
			break;
			
		// Data 송신 => NACK 수신 => 송신 종료
		case TWSR_ST_DATA_NACK:
			tx_idx = 0;
			TWCR = 1 << TWINT | 1 << TWEA | 1 << TWEN | 1 << TWIE;   
			break;
			
		 // 마지막 데이터 송신 + ACK 수신
		case TWSR_ST_LAST_DATA:
			// 전송 완료 : 버스 해체, 다음 주소 대기
			tx_idx = 0;
			TWCR = 1 << TWINT | 1 << TWEA | 1 << TWEN | 1 << TWIE;
			break;
			
		// Error 복구 TWINT clear, ACK 재활성화
		default:
			TWCR = 1 << TWINT | 1 << TWEA | 1 << TWEN | 1 << TWIE;   
			break;
	}
}

void i2c_slave_main()
{
	init_slave_i2c();
	sei();
	
	while(1)
	{
	}
}

/* TWI(I2C) Driver */
void init_slave_i2c(void)
{
	// TWAR: (TWI Address Register)
	TWAR = SLAVE_ADDRESS << 1;                        // 7-bit 설정
	TWCR = 1 << TWEA | 1 << TWEN | 1 << TWIE;         // ACK, INT 활성화
}


















