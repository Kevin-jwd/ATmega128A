/*
 * i2c_master.c
 *
 * Created: 2026-06-30 오전 7:47:49
 *  Author: user
 */ 

#if 1
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
#define SLAVE_ADDR   0x50      /* 7-bit Slave 주소 */
#define I2C_FREQ     100000UL  /* 100 kHz */

/* TWBR = (F_CPU / I2C_FREQ - 16) / 2  (Prescaler=1) */
#define TWBR_VALUE   ((F_CPU / I2C_FREQ - 16) / 2)  // TWBR(TWI Bit Rate Register) 

/* TWI 상태 코드 */
// TWSR(TWI Status Register):7번~3번 bit(5개 비트) : TWI의 전송  상태나 오류 등을 나타 낸다. 
#define TWSR_START     0x08   // START 조건 전송 완료
#define TWSR_REP_START 0x10   // TWSR_REP_START (0x10) - Repeated START
/*
일반 START와 Repeated START의 차이
일반 START:
[STOP] ─── 버스 유휴 상태 ─── [START] ─── 새 트랜잭션

Repeated START:
[START] ─ SLA+W ─ DATA ─ [Re-START] ─ SLA+R ─ DATA ─ [STOP]
↑ STOP 없이 바로 새 START
왜 필요한가?
I2C 버스는 Multi-Master 환경이 가능하다. STOP을 보내는 순간 버스가 해제되어 다른 Master가 버스를 가로챌 수 있다.
Master A: [START] ─ SLA+W ─ DATA ─ [STOP]  ← 버스 해제!
                                              ↑ 여기서 Master B가 끼어들 수 있음
Master A:                                       [START] ─ SLA+R ...

Repeated START를 쓰면:
Master A: [START] ─ SLA+W ─ DATA ─ [Re-START] ─ SLA+R ─ DATA ─ [STOP]
↑ 버스를 계속 점유하면서 방향 전환
*/
#define TWSR_MT_SLA_ACK  0x18  // SLA+W 전송 후 ACK 수신
#define TWSR_MT_DATA_ACK 0x28  // 데이터 전송 후 ACK 수신
#define TWSR_MR_SLA_ACK  0x40  // SLA+R 전송 후 ACK 수신 
#define TWSR_MR_DATA_ACK 0x50  // 데이터 수신 후 ACK 전송
#define TWSR_MR_DATA_NACK 0x58 // 데이터 수신 후 NACK 전송 (마지막)
/*
ATmega128A TWI 관련 레지스터:
TWCR : TWI Control Register
TWSR : TWI Status Register  (상위 5bit = 상태코드, 하위 2bit = Prescaler)
TWBR : TWI Bit Rate Register
TWDR : TWI Data Register
TWAR : TWI Address Register (Slave 모드용)

TWCR 레지스터 각 비트 역활
TWINT : 인터럽트 플래그 (1쓰면 clear (일반 flag와 반대)
TWEA : ACK enable (1 자동 ACK, 0=NACK)
TWSTA : START 발생 (MASTER전용)
TWSTO : STOP 발생 (MASTER전용)
TWEN  : TWI Enable  I2C 하드웨어 활성화 
TWIE  : 인터럽트Enable TWI 인터럽트 활성화
*/

int i2c_master_main(void); 

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
    TWCR = 1 << TWINT | 1 << TWEN;
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
// 수신중: Master--> Slave에게 ACK --> 다음 Byte 계속 보내
// 마지막: Master--> Slave에게 NACK --> "여기서 끝, 버스 놓아줘" 
/* ─────────────────────────────────────────
 *  루프백 테스트: "ABC" 전송 → 수신
 * ───────────────────────────────────────── */
void loopback_test(void)
{
    char tx_msg[] = "ABC";
    char       rx_buff[4] = {0};
    uint8_t    status;

    /* ── 1단계: Master → Slave 전송 ── */
    status = i2c_start();
    if (status) { printf("[ERR] START failed\r\n"); return; }

    status = i2c_slave_addr_send((SLAVE_ADDR << 1) | 0); /* 0xA0 (SLA+W) */
    if (status != TWSR_MT_SLA_ACK) { printf("[ERR] SLA+W NACK\r\n"); i2c_stop(); return; }

    for (uint8_t i = 0; tx_msg[i] != '\0'; i++) {
        status = i2c_write((uint8_t)tx_msg[i]);
        if (status != TWSR_MT_DATA_ACK) {
            printf("[ERR] DATA write NACK\r\n");
            i2c_stop();
            return;
        }
    }
    i2c_stop();
    // STRAT 0xA0         ACK 'A' ACK 'B' ACK 'C' ACK STOP
	//       어드레스write
    _delay_ms(5);   /* Slave ISR 처리시간 확보 Slave가 수신 데이터를 처리할 시간 */

    /* ── 2단계: Slave → Master 수신 (Repeated START or new START) ── */
    status = i2c_start();
    if (status) { printf("[ERR] START(read) failed\r\n"); return; }

    status = i2c_slave_addr_send((SLAVE_ADDR << 1) | 1); /* 0xA1 SLA+R */
    if (status != TWSR_MR_SLA_ACK) { printf("[ERR] SLA+R NACK\r\n"); i2c_stop(); return; }

    /* 3바이트 수신 (A, B, C) */
    rx_buff[0] = i2c_read_ack();  // 'A' 수신 + ACK전송('B'도 보내줘)
    rx_buff[1] = i2c_read_ack();  // 'B' 수신 + ACK전송('C'도 보내줘)
    rx_buff[2] = i2c_read_nack(); // 'C' 수신 +  NACK(마지막: 여가서  끝내자) 전송 */
	// NAK 없이 STOP만 보내면 안되나  ?
	// 안됨. Salve는 Master가 ACK를보낼것으로기대 하면 다음 바이트를 분지. NACK없이 갑자기 STOP을 보내면
	// Slave가  버스 상태를  잘못 해석 할 수 있다.  
	// 일부Salve Device(EEPROM)은 다음 내부 동작으로넘어가지못하고 버스를붙잡는현상  발생 가능 
	// I2C space상 마지막 바이트에 NACK 후 STOP 보내는것이 정상적인 절차  
    i2c_stop();   // STOP 

    /* ── 결과 출력 ── */
    printf("TX: ");
    printf(tx_msg);
    printf("  RX: ");
    printf(rx_buff);
    printf("\n");

    if (rx_buff[0] == tx_msg[0] &&
        rx_buff[1] == tx_msg[1] &&
        rx_buff[2] == tx_msg[2]) {
        printf(">> Loopback OK!\n");
    } else {
        printf(">> Loopback MISMATCH!\n");
    }
}

/* ─────────────────────────────────────────
 *  main
 * ───────────────────────────────────────── */
int i2c_master_main(void)
{
    init_i2c();

    printf("=== I2C Master Loopback Test ===\r\n");

    while (1) {
        loopback_test();
        _delay_ms(1000); /* 1초마다 반복 */
    }

    return 0;
}
#endif 