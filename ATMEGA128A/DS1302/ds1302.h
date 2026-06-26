/*
 * ds1302.h
 *
 * Created: 2026-06-26 오후 2:41:55
 *  Author: kccistc
 */ 


#ifndef DS1302_H_
#define DS1302_H_

#define F_CPU 16000000UL  //16MHz
#include <avr/io.h>  // PORTA PORTB PORTD... IO관련 reg가 들어 있다.
#include <util/delay.h>  // _delay_ms _delay_us 등
#include <stdio.h>
#include <string.h>

/* Common */
#define DS1302_DDR       DDRF
#define DS1302_PORT      PORTF

/* Pin Number */
#define DS1302_CLK       0
#define DS1302_DAT       1 
#define DS1302_DAT_PIN   PINF            // To read value
#define DS1302_RST       2

/* WRITE ADDRESS */
#define SEC_WRITE_ADDRESS     0x80       // 초
#define MIN_WRITE_ADDRESS     0x82		 // 분
#define HOUR_WRITE_ADDRESS    0x84		 // 시
#define DATE_WRITE_ADDRESS    0x86		 // 일
#define MONTH_WRITE_ADDRESS   0x88		 // 월
#define DAY_WRITE_ADDRESS     0x8A		 // 요일
#define YEAR_WRITE_ADDRESS    0x8C		 // 년

/* READ ADDRESS */
#define SEC_READ_ADDRESS      0x81       // 초
#define MIN_READ_ADDRESS      0x83		 // 분
#define HOUR_READ_ADDRESS     0x85		 // 시
#define DATE_READ_ADDRESS     0x87		 // 일
#define MONTH_READ_ADDRESS    0x89		 // 월
#define DAY_READ_ADDRESS      0x8B		 // 요일
#define YEAR_READ_ADDRESS     0x8D		 // 년

typedef struct _ds1302
{
	uint8_t seconds;       // 초
	uint8_t minutes;	   // 분
	uint8_t hours;		   // 시
	uint8_t date;		   // 일
	uint8_t month;		   // 월
	uint8_t day;		   // 요일 (1: sun, 2: mon, 3: thue, 4: wen, 5: thur, 6: fri, 7: sat)
	uint8_t year;		   // 년
	uint8_t ampm;          // 오전/오후 (1: pm, 0: am)
	uint8_t hour_mode;     // 12/24 (1: 12, 0: 24)
} t_ds1302;

#endif /* DS1302_H_ */