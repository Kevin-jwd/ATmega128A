#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>

// --- 모터 드라이버 전체 핀 정의 ---
#define MOTOR_PORT  PORTF
#define MOTOR_DDR   DDRF
#define IN1         PF0  // 좌측 방향 제어 1
#define IN2         PF1  // 좌측 방향 제어 2
#define IN3         PF2  // 우측 방향 제어 1
#define IN4         PF3  // 우측 방향 제어 2

#define EN_DDR      DDRB
#define ENA         PB5  // 좌측 모터 활성화 (100% 출력용 디지털 핀 사용)
#define ENB         PB6  // 우측 모터 활성화 (100% 출력용 디지털 핀 사용)

int main(void) {
	// 1. 모든 방향 제어 핀 및 활성화 핀을 출력(Output) 모드로 설정
	MOTOR_DDR |= (1 << IN1) | (1 << IN2) | (1 << IN3) | (1 << IN4);
	EN_DDR    |= (1 << ENA) | (1 << ENB);

	while (1) {
		// --- [STEP 1] 1초 동안 전체 바퀴 전진 ---
		// 좌측 정방향(IN1=1, IN2=0) 및 우측 정방향(IN3=1, IN4=0) 세팅
		MOTOR_PORT = (1 << IN1) | (0 << IN2) | (1 << IN3) | (0 << IN4);
		
		// ENA, ENB 모두 High(5V) 인가
		EN_DDR |= (1 << ENA) | (1 << ENB);
		PORTB  |= (1 << ENA) | (1 << ENB);
		
		_delay_ms(1000); // 1초 동안 전진 유지


		// --- [STEP 2] 1초 동안 전체 바퀴 정지 ---
		// ENA, ENB 신호를 Low(0)로 떨어뜨려 양쪽 모터 전류 차단
		PORTB &= ~((1 << ENA) | (1 << ENB));
		
		// 안전을 위해 모든 방향 제어 핀도 Low로 하강
		MOTOR_PORT = 0x00;
		
		_delay_ms(1000); // 1초 동안 정지 유지
	}
}