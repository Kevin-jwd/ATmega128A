/*
 * i2c_master.c
 *
 * Created: 2026-06-30 오전 7:47:49
 *  Author: user
 */ 

/*
 * ATmega128A I2C Master
 * "ABC" 문자열을 Slave로 전송하고, Slave가 돌려보낸 데이터를 수신하여
 * UART로 출력하는 루프백 테스트 프로그램
 *
 * 하드웨어:
 *   PD0 (SCL) ─ 4.7kΩ ─ VCC
 *   PD1 (SDA) ─ 4.7kΩ ─ VCC
 *   두 ATmega128A의 GND 공통 연결
 *
 * F_CPU: 16MHz, I2C 속도: 100kHz, UART: 9600 bps
 *  ACK  vs NACK 의 의미
 *  신호    SDA    Slave 입장에서의 의미
 *  ====  ===== ==========================
 *  ACK     LOW   다음 바이트도 계속 보내
 *  NACK    HIGH  이제 그만 보내라 전송끝 
 */

#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>

/* ───────── I2C 설정 ───────── */
#define SLAVE_ADDR   0x68      /* 7-bit Slave 주소 */
#define I2C_FREQ     100000UL  /* 100 kHz */

/* TWBR = (F_CPU / I2C_FREQ - 16) / 2  (Prescaler=1) */
#define TWBR_VALUE   ((F_CPU / I2C_FREQ - 16) / 2)  // TWBR(TWI Bit Rate Register) 

/* TWI 상태 코드 */
// TWSR(TWI Status Register):7번~3번 bit(5개 비트) : TWI의 전송  상태나 오류 등을 나타 낸다. 
#define TWSR_START     0x08   // START 조건 전송 완료
#define TWSR_REP_START 0x10   // TWSR_REP_START (0x10) - Repeated START

#define TWSR_MT_SLA_ACK  0x18  // SLA+W 전송 후 ACK 수신
#define TWSR_MT_DATA_ACK 0x28  // 데이터 전송 후 ACK 수신
#define TWSR_MR_SLA_ACK  0x40  // SLA+R 전송 후 ACK 수신 
#define TWSR_MR_DATA_ACK 0x50  // 데이터 수신 후 ACK 전송
#define TWSR_MR_DATA_NACK 0x58 // 데이터 수신 후 NACK 전송 (마지막)

#define TIMEKEEPER_REGISTER_SIZE   8

uint8_t timekeeper_register_data[7] = { 0 };

int i2c_master_main(void); 
uint8_t bcd2dec(uint8_t data);
void init_time(void);

/* ─────────────────────────────────────────
 *  TWI (I2C) 드라이버
 * ───────────────────────────────────────── */
void init_i2c(void)
{
    TWSR = 0x00;              /* Prescaler = 1 */
    TWBR = (uint8_t)TWBR_VALUE;  // SCL 주파수 결정 register
}

/* 공통 대기: TWINT 세트 대기 */
// TWI 하드웨어가 동작을 완료하면 TWINT 플래그를 1로 세트. 
// 소프트웨어는 이것을 폴링하며 대기. TWCR에 TWINT=1을 쓰면 클리어됨 (일반 플래그와 반대)
static void twi_wait(void)
{
    while (!(TWCR & (1 << TWINT)));
}

/* START 조건 */
uint8_t i2c_start(void)
{
    TWCR = 1 << TWINT | 1 << TWSTA | 1 << TWEN;
	//        TWINT 클리어       START요청     TWI활성화 
    twi_wait();   // WI 하드웨어가 동작을 완료하면 TWINT 플래그를 1로 세트. 
    uint8_t status = TWSR & 0xF8;
    return (status == TWSR_START || status == TWSR_REP_START) ? 0 : status;
	//  TW_START: START 조건 전송 완료    성공이면: 0 , 실패면 에러상태 코드 반환 
}

/* STOP 조건 */
void i2c_stop(void)
{
    TWCR = 1 << TWINT | 1 << TWSTO | 1 << TWEN;
    /* STOP 후 TWINT는 자동으로 클리어되지 않으므로 짧게 대기 */
    _delay_us(10);
}

/* SLA+W / SLA+R 전송 */
uint8_t i2c_slave_addr_send(uint8_t addr_rw)
{
    TWDR = addr_rw;
    TWCR = 1 << TWINT | 1 << TWEN | 0 << TWSTA;
    twi_wait();
    return TWSR & 0xF8;
}

/* 데이터 바이트 전송 */
uint8_t i2c_write(uint8_t data)
{
    TWDR = data;
    TWCR = 1 << TWINT | 1 << TWEN;
    twi_wait();
    return TWSR & 0xF8; /* TWSR_MT_DATA_ACK(0x28) 이면 성공 */
}

/* 데이터 바이트 수신 –> ACK 전송 */
uint8_t i2c_read_ack(void)
{
    TWCR = 1 << TWINT | 1 << TWEN | 1 << TWEA;  // TWEA=1  ACK 자동전송 
    twi_wait();
    return TWDR;
}

/* 데이터 바이트 수신 –> NACK 전송 (Slave에게 종료 알림) */
uint8_t i2c_read_nack(void)
{
    TWCR = 1 << TWINT | 1 << TWEN;     // TWEA=0 : NACK 전송 
    twi_wait();
    return TWDR;
}

// [초, 분, 시, 요일, 일, 월, 년] 순서로 BCD 포맷 장전
// ★ 초(Second) 자리에 0x00을 밀어 넣어야 CH 비트가 0으로 클리어되면서 시계가 가동합니다.
uint8_t initial_time[7] = {
	0x00, // Second : 00초 (CH 비트 0으로 클리어)
	0x10, // Minute : 10분
	0x17, // Hour   : 17시 (24시간제 변환값)
	0x04, // Day    : 4 (수요일 - 1:일, 2:월, 3:화, 4:수...)
	0x01, // Date   : 1일
	0x07, // Month  : 7월
	0x26  // Year   : 26년 (2026년을 의미)
};

void init_time(void)
{
	uint8_t status;
	
	// 1. start 전송
	status = i2c_start();
	if(status)
	{
		printf("Error : START failed !!!!!\n");          // START 조건 전송 실패
		return;
	}
	
	// 2. SLAVE_ADDR + W (0xD0) 전송
	status = i2c_slave_addr_send(0x68 << 1); // 0x68을 좌측 시프트하여 0xD0 전송
	if((status & 0xF8) != 0x18) // 0x18 = TWSR_MT_SLA_ACK
	{
		printf("Error : SLA + W Not ACK !!!!! (st: %x)\n", status);
		i2c_stop();
		return;
	}
	
	// 3. 데이터를 쓰기 시작할 메모리 포인터 주소 (0x00 = 초 레지스터) 전송
	// 하드웨어 트리거를 위해 이전에 만드신 i2c_data_write 또는 i2c_slave_addr_send 함수를 활용합니다.
	status = i2c_slave_addr_send(0x00);
	if((status & 0xF8) != 0x28) // 0x28 = TWSR_MT_DATA_ACK
	{
		printf("Error : Register Address 0x00 Not ACK !!!!! (st: %x)\n", status);
		i2c_stop();
		return;
	}

	// 4. 실제 시간 데이터 7바이트 연속으로 쓰기
	// 내부 포인터가 자동으로 0x00부터 0x06까지 1씩 증가하며 칩에 박힙니다.
	for(int i = 0; i < 7; i++)
	{
		// 데이터 전송 후 ACK(0x28) 확인
		status = i2c_slave_addr_send(initial_time[i]);
		if((status & 0xF8) != 0x28) // 0x28 = TWSR_MT_DATA_ACK
		{
			printf("Error : DATA [%d] Not ACK !!!!! (st: %x)\n", i, status);
			i2c_stop();
			return;
		}
	}

	// 5. STOP 신호 전송 (★ 이 순간부터 칩이 깨어나 17:10:00 기준으로 카운트를 시작합니다)
	i2c_stop();
	printf("RTC Initialization Success! 2026-07-01 17:10:00 Started.\n");
}
void read_time()
{
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
	// Master -> Slave 레지스터 주소 설정
	// 메모리 포인터 설정 0x00 ~ 0x06 -> seconds, minutes, hours, day, date, month, year
	status = i2c_slave_addr_send(SLAVE_ADDR << 1);
	
	status = i2c_write(0x00);
	
	// 3. Repeat Start
	i2c_start();
	
	// 2. SLAVE_ADDR + R 전송
	// Master -> Slave 읽을 주소 설정
	status = i2c_slave_addr_send(SLAVE_ADDR << 1 | 1);     // SLA + R
	
	if(status != TWSR_MR_SLA_ACK)
	{
		printf("Error : SLA + Not ACK !!!!!\n");
		i2c_stop();
		return;
	}

	// 3. read DATA
	for(int i = TIMEKEEPER_REGISTER_SIZE-1-1;i > 0;i--)
	{
		timekeeper_register_data[i] = bcd2dec(i2c_read_ack());
	}
	timekeeper_register_data[0] = bcd2dec(i2c_read_nack());
	
	// 4. STOP signal
	i2c_stop();
	
	// 5. 대기
	_delay_ms(5);          // Slave 대기 시간
}

uint8_t bcd2dec(uint8_t data)
{
	uint8_t high, low;
	
	low = data & 0x0f;
	high = (data >> 4) * 10;
	
	return(high+low);
}