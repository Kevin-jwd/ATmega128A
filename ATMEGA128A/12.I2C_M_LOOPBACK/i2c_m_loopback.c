/*
 * i2c_m_loopback.c
 *
 * Created: 2026-06-30 오전 10:52:42
 *  Author: kccistc
 */ 

/* "ABC" 문자열을 Slave로 전송하고 돌려보낸 문자열을 수신하여 UART로 출력하는 loop-back test program을 작성한다
*  ATmega128a x 2ea : master 1, slave 1
*
*  H/W 조건
*  ==============
*  1. I2C 연결
*  PD0 (SCL)  --- 10k (pull-up 저항) --- VCC (pull-up 저항은 master만 연결)
*  PD1 (SDA)  --- 10k (pull-up 저항) --- VCC
*  두 atmega의 GND는 공통으로 연결 (VCC끼리는 연결 안해도 됨)
*
*  I2C Address      : 0x60 (0110 0000)
*  1 bit left shift : 0xC0 (1100 0000)
*
*
*  2. 속도
*  F_CPU: 16000000Hz(16MHz), I2C: 100000Hz(100kHz), UART: 9600ps
*
*  3. Handshaking
*
*    신호         SDA          Slave 입장에서의 의미
*  ===========================================================================
*    ACK          LOW          다음 바이트도 계속 보냄. 정상 수신 의미도 있음.
*    NACK         HIGH         전송 끝
*
*/

#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <string.h>

/* I2C settings */
#define SLAVE_ADDRESS     0x50  // 7bit slave address
#define I2C_FREQUENCY 100000UL  // 100kHz
// TWBR(Two Wire Bit Rate Register) 
// TWBR = (F_CPU / I2C_FREQUENCY -16) / 2    (분주비 = 1)
#define TWBR_VALUE (F_CPU / I2C_FREQUENCY -16) / 2   

//------------- TWI Status code (Master) -------------
// TWSR (TWI Status register) : bit 7~3 (5) ==> I2C통신의 전송 상태나 오류를 나타냄
/* Master Transmitter */
#define TWSR_MT_START              0x08      // START 조건 전송 완료
#define TWSR_MT_REPEAT_START       0x10      
#define TWSR_MT_SLA_ACK            0x18      // Master가 SLA + W 송신 후 ACK 수신
#define TWSR_MT_SLA_NACK           0x20      // Master가 SLA + W 송신 후 NACK 수신
#define TWSR_MT_DATA_ACK           0x28      // Master가 Data 송신 후 ACK 수신
#define TWSR_MT_DATA_NACK          0x30      // Master가 Data 전송 후 NACK 수신
#define TWSR_MT_ARBITRATION_LOST   0x38      // 우선순위 경쟁 패배

/* Master Receiver */
#define TWSR_MR_START              0x08
#define TWSR_MR_REPEAT_START       0x10   
#define TWSR_MR_ARBITRATION_LOST   0x38
#define TWSR_MR_SLA_ACK            0x40       // Master가 SLA + R 송신 후 ACK 수신
#define TWSR_MR_SLA_NACK           0x48       // Master가 SLA + R 송신 후 NACK 수신
#define TWSR_MR_DATA_ACK           0x50
#define TWSR_MR_DATA_NACK          0x58       // 데이터 수신 후 NACK 송신

/* Slave Receiver */
//#define TWSR_SR_START              0x60
//#define TWSR_SR_REPEAT_START       0x68
//#define TWSR_SR_ARBITRATION_LOST   0x70
//#define TWSR_SR_SLA_ACK            0x78
//#define TWSR_SR_SLA_NACK           0x80
//#define TWSR_SR_DATA_ACK           0x88
//#define TWSR_SR_DATA_NACK          0x90
//#define TWSR_SR_         
//#define TWSR_SR

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

void i2c_master_main(void);
void init_i2c(void);
void loopback_test(void);
uint8_t i2c_start(void);
uint8_t i2c_slave_addr_send(uint8_t addr_rw);
uint8_t i2c_stop(void);
uint8_t i2c_data_write(uint8_t data);
uint8_t i2c_data_read_acksend(void);
uint8_t i2c_data_read_nacksend(void);

void i2c_master_main()
{
	init_i2c();
	sei();
	
	while(1)
	{
		loopback_test();
	}
}

/* TWI(I2C) Driver */
void init_i2c(void)
{
	TWSR = 0x00;
	TWBR = TWBR_VALUE;      // SCL 주파수 결정 register 100kHz
}


void loopback_test(void)
{
	char tx_msg[] = "ABC";
	char rx_buff[4] = { 0 };
	uint8_t status;
	
	// ================ PHASE 1 ==================
	// ========== Master --> Slave 전송 ==========
	// ===========================================
	
	// 1. start 전송
	status = i2c_start();
	
	if(status)
	{
		printf("Error : START failed !!!!!\n");          // START 조건 전송 실패
		return;
	}
	
	// 2. SLAVE_ADDR + W 전송
	status = i2c_slave_addr_send(SLAVE_ADDRESS << 1);     // 0xC0 : SLA + W
	if(status != TWSR_MT_SLA_ACK)
	{
		printf("Error : SLA + Not ACK !!!!!\n");
		i2c_stop();
		return;
	}
	
	// 3. write DATA 3 bytes 
	for(int i = 0;tx_msg[i] != '\0';i++)
	{
		status = i2c_data_write(tx_msg[i]);
		if(status != TWSR_MR_DATA_ACK)
		{
			printf("Error : DATA + Not ACK !!!!!\n");
			i2c_stop();
			return;
		}
	}
	
	// 4. STOP signal
	i2c_stop();
	
	// 5. 대기
	_delay_ms(5);          // Slave 대기 시간
	
	// ================ PHASE 2 ==================
	// ========== Master <-- Slave 전송 ==========
	// ===========================================
	
	// 1. start 전송
	status = i2c_start();
	if(status)
	{
		printf("Error : Read START failed !!!!!\n");
		return;
	}
	// 2. SLAVE_ADDR + R 전송
	status = i2c_slave_addr_send(SLAVE_ADDRESS << 1 | 1);     // 0xC1 : SLA + R
	if(status != TWSR_MR_SLA_ACK)
	{
		printf("Error : SLA + Not ACK !!!!!\n");
		i2c_stop();
		return;
	}
	
	// 3. 3bytes 수신 A, B, C => ACK
	rx_buff[0] = i2c_data_read_acksend();       // 'A' 수신 + ACK 송신('B' 수신 준비)
	rx_buff[1] = i2c_data_read_acksend();       // 'B' 수신 + ACK 송신('C' 수신 준비)
	rx_buff[2] = i2c_data_read_nacksend();       // 'C' 수신 + NACK 송신(수신 종료)
	
	// 4. i2c stop
	i2c_stop();
	
	// ================ 결과 출력 ================
	printf("Tx: %s, Rx: %s\n", tx_msg, rx_buff);
	if(strncmp(rx_buff, tx_msg, 3) == 0)
	{
		printf("LOOP BACK OK !!!!!!\n");
	}
	else
	{
		printf("LOOP BACK FAILED !!!!!!\n");
	}
}

uint8_t i2c_start()
{
	
	TWCR = 1 << TWINT | 1 << TWSTA | 1 << TWEN;
	// TWINT        TWSTA: Start 요청   TWEN: I2C HW 활성화
	// I2C HW가 동작이 완료될때까지 기다림. 동작이 완료되면 TWINT flag가 1로 set됨
	twi_wait();
	
	uint8_t st = TWSR & 0xf8;                       // 7~3bit 까지 값을 취함 (상위 5비트 상태, 하위 2비트 분주)
	
	return (st == TWSR_MT_START) ? 0 : st;  // 성공이면 0, 아니라면 에러 상태 코드 반환
}

/* 공통 대기: TWINT 세트 대기 */
// TWI 하드웨어가 동작 완료하면 TWINT 플래그를 1로 세트
// 소프트웨어는 이것을 폴링하며 대기, TWCR에 TWINT = 1 을 쓰면 clear됨
static void twi_wait(void)
{
	while(!(TWCR & (1 << TWINT)));
}

/* SLAVE_ADDR + R/W 전송 */
uint8_t i2c_slave_addr_send(uint8_t addr_rw)
{
	// 전송 전 : TWINT 플래그 1 
	// 전송 시작 신호 : TWCR = 1 << TWINT (트리거로 알아듣고 전송 시작과 동시에 TWINT 플래그를 0으로 clear) 
	// 전송 중 : TWINT 플래그 0 (twi_wait()가 이때 대기)
	// 전송 완료 : TWINT 플래그 1 (HW가 자동으로 1으로 값을 올리면서 twi_wait() 탈출)
	uint8_t st;
	
	TWDR = addr_rw;
	TWCR = 1 << TWINT | 1 << TWEN;
	//         TWINT         TWEN: I2C HW 활성화
	// TWINT 플래그 1로 set하면 HW적으로는 0으로 클리어 => 동작 시작 신호로 받아들임
	
	// I2C HW가 동작이 완료될때까지 기다림. 동작이 완료되면 TWINT flag가 1로 set됨
	// 전송이 완료되면 하드웨어가 다시 TWINT 플래그를 세워(1로 만듦) twi_wait() 루프를 탈출하게 됩니다.
	twi_wait();
	st = TWSR & 0xf8;                       // 7~3bit 까지 값을 취함
	
	return st;
}

/* STOP 조건 */
void i2c_stop(void)
{	
	TWCR = 1 << TWINT | 1 << TWSTO | 1 << TWEN;
	// STOP 후 TWINT clear되기까지 대기
	_delay_us(10);
}

/* 데이터 바이트 전송 */
uint8_t i2c_data_write(uint8_t data)
{		
	TWDR = data;
	TWCR = 1 << TWINT | 1 << TWEN;
	//         TWINT         TWEN: I2C HW 활성화
	// I2C HW가 동작이 완료될때까지 기다림. 동작이 완료되면 TWINT flag가 1로 set됨
	twi_wait();
		
	return (TWSR & 0xf8);          // 0b1111 1000
}

/* 데이터 바이트 수신 -> ACK 전송 */
uint8_t i2c_data_read_acksend()
{
	TWCR = 1 << TWINT | 1 << TWEN | 1 << TWEA;        // TWEA = 1; ACK 자동 전송
	//         TWINT         TWEN: I2C HW 활성화
	// I2C HW가 동작이 완료될때까지 기다림. 동작이 완료되면 TWINT flag가 1로 set됨
	twi_wait();
	
	return TWDR;
}

/* 데이터 바이트 수신 -> NACK 전송 (Slave에게 종료 알림) */
uint8_t i2c_data_read_nacksend()
{
	TWCR = 1 << TWINT | 1 << TWEN;                     // TWEA = 0; NACK 자동 전송
	//         TWINT         TWEN: I2C HW 활성화
	// I2C HW가 동작이 완료될때까지 기다림. 동작이 완료되면 TWINT flag가 1로 set됨
	twi_wait();
	
	return TWDR;
}













