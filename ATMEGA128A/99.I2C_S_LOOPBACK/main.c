/*
 * 99.I2C_S_LOOPBACK.c
 *
 * Created: 2026-06-28 오후 3:57:45
 * Author : user
 */ 
#if 1
/*
 * ATmega128A I2C Slave
 * Master로부터 "ABC"를 받아서 그대로 돌려보내는 루프백 프로그램
 * TWI Slave 인터럽트 방식
 *
 * 하드웨어:
 *   PD0 (SCL) ─ 공통 버스
 *   PD1 (SDA) ─ 공통 버스
 *   (풀업 저항은 Master 쪽에 장착)
 *
 * F_CPU: 16MHz
 */

#define F_CPU 16000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <string.h>

/* ───────── I2C 설정 ───────── */
#define SLAVE_ADDR   0x50   /* Master와 동일한 7-bit 주소 */

/* TWI 상태 코드 (Slave 전용) */
// 아래의 상태 코드를 수신하면 ISR(TWI_vect) 상태로 분기  한다. 
#define TW_SR_SLA_ACK    0x60  /* Slave Receive: SLA+W 수신, ACK 전송 --> 버퍼초기화 */
#define TW_SR_DATA_ACK   0x80  /* Slave Receive: 데이터 수신, ACK 전송 -> rx_buf에 저장 */
#define TW_SR_STOP       0xA0  /* STOP / Repeated START 감지 -->  tx_idx리셋 */
#define TW_ST_SLA_ACK    0xA8  /* Slave Transmit: SLA+R 수신, ACK 전송 --> 첫 바이트 준비 */
#define TW_ST_DATA_ACK   0xB8  /* Slave Transmit: 데이터 전송, ACK 수신 --> 다음 바이트 송신 */
#define TW_ST_DATA_NACK  0xC0  /* Slave Transmit: 데이터 전송, NACK 수신 (마지막) -->  송신 종료 */
#define TW_ST_LAST_DATA  0xC8  /* @@@@ Slave Transmit: 마지막 데이터 전송, ACK 수신 */
							// // Master가 ACK지만 Slave가 이미 마지막 선언 → 동일하게 종료 처리
							// 0xC8은 "Slave가 마지막 바이트라고 선언(TWEA=0)했지만, Master는 그 바이트에 ACK를 보낸 경우" 발생하며, 
							// 실제 하드웨어에서 Master 구현 방식에 따라 충분히 발생할 수 있는 정상적인 상태코드다.
/*
tx_idx=0 → 'A' 전송, TWEA=1  → Master ACK  → 0xB8
tx_idx=1 → 'B' 전송, TWEA=1  → Master ACK  → 0xB8
tx_idx=2 → 'C' 전송, TWEA=0  → Master ACK  → 0xC8 ★ 발생!
                             → Master NACK → 0xC0
*/

/* ───────── 내부 버퍼 ───────── */
#define BUF_SIZE 8

static volatile uint8_t rx_buf[BUF_SIZE];    // 컴파일러 최적화 방지: volatile 
static volatile uint8_t rx_len  = 0;    /* 수신된 바이트 수 */
static volatile uint8_t tx_idx  = 0;    /* 송신 인덱스 */

/* ─────────────────────────────────────────
 *  TWI 초기화 (Slave 모드)
 * ───────────────────────────────────────── */
void i2c_slave_init(void)
{
    TWAR  = (SLAVE_ADDR << 1);          /* TWAR: Address Register 7-bit 주소 설정(0x50), 브로드캐스트 응답 비활성 */
    TWCR  = (1 << TWEA) | (1 << TWEN) | (1 << TWIE); /* ACK 활성화, Interrupt 활성화 */
}

/* ─────────────────────────────────────────
 *  TWI 인터럽트 서비스 루틴
 * ───────────────────────────────────────── */
ISR(TWI_vect)
{
    uint8_t status = TWSR & 0xF8;

    switch (status)
    {
    /* ── Slave Receive 단계 ── */
    case TW_SR_SLA_ACK:   // SLA 주소인식
        /* SLA+W 수신: 버퍼 초기화 후 ACK */
        rx_len = 0;    // 버퍼 초기화 
        TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWIE) | (1 << TWEA);
        break;

    case TW_SR_DATA_ACK:     // 데이터 수신 (반복)
        /* 데이터 1바이트 수신 */
        if (rx_len < BUF_SIZE) {
            rx_buf[rx_len++] = TWDR;
        }
        TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWIE) | (1 << TWEA);
        break;

    case TW_SR_STOP:   // STOP 감지 (수신 완료)
        /* STOP 감지: 수신 완료. 다음 요청을 위해 대기 */
        /* tx_idx를 리셋해서 Read 요청에 처음부터 돌려줄 수 있게 함 */
        tx_idx = 0;
        TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWIE) | (1 << TWEA);   // 읽기 트랜잭션 대기 
        break;

    /* ── Slave Transmit 단계 ── */
	// 0xA8 --> 0xB8 --> 0xC0 
    case TW_ST_SLA_ACK:   
        /* SLA+R 수신: 첫 번째 바이트 준비 */
        tx_idx = 0;
        /* fall through */

    case TW_ST_DATA_ACK:
        /* ACK를 받았으므로 다음 바이트 전송 */
        if (tx_idx < rx_len) {
            TWDR = rx_buf[tx_idx++];
        } else {
            TWDR = 0xFF; /* 버퍼 이상: 더미 */
        }
        /* 마지막 바이트이면 TWEA=0으로 NACK 준비 */
        if (tx_idx >= rx_len) {
            TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWIE);  // 마지막이면 TWEA=0 (NACK준비)
        } else {
            TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWIE) | (1 << TWEA);  // 아니면 TWEA=1 (계속 ACK) 
        }
        break;

    case TW_ST_DATA_NACK:       // NACK 받음 --> 송신 종료
    case TW_ST_LAST_DATA:
/*
  Master가 ACK지만 Slave가 이미 마지막 선언 → 동일하게 종료 처리
  0xC8은 "Slave가 마지막 바이트라고 선언(TWEA=0)했지만, Master는 그 바이트에 ACK를 보낸 경우" 발생하며, 
  실제 하드웨어에서 Master 구현 방식에 따라 충분히 발생할 수 있는 정상적인 상태코드다.
  tx_idx=0 → 'A' 전송, TWEA=1  → Master ACK  → 0xB8
  tx_idx=1 → 'B' 전송, TWEA=1  → Master ACK  → 0xB8
  tx_idx=2 → 'C' 전송, TWEA=0  → Master ACK  → 0xC8 ★ 발생!
                               → Master NACK → 0xC0
*/
        /* 전송 완료: 버스 해제, 다음 주소 대기 */
        tx_idx = 0;
        TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWIE) | (1 << TWEA);
        break;

    default:
        /* 에러 복구: TWINT 클리어, ACK 재활성화 */
        TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWIE) | (1 << TWEA);
        break;
    }
}

/* ─────────────────────────────────────────
 *  main
 * ───────────────────────────────────────── */
int main(void)
{
    i2c_slave_init();
    sei();  /* 전역 인터럽트 활성화 */

    while (1) {
        /* 모든 처리는 ISR에서 수행 */
        /* 필요 시 rx_buf, rx_len 을 활용한 추가 처리 가능 */
    }

    return 0;
}

#endif 
