/*
 * fnd.h
 *
 * Created: 2026-06-12 오후 1:32:56
 *  Author: kccistc
 */ 


#ifndef FND_H_
#define FND_H_

#define F_CPU 16000000UL
#include <avr/io.h>

#define FND_DATA_DDR DDRC
#define FND_DATA_PORT PORTC

#define FND_CONTROL_DDR DDRB
#define FND_CONTROL_PORT PORTB

#define FND_CONTROL_D1 4
#define FND_CONTROL_D2 5
#define FND_CONTROL_D3 6
#define FND_CONTROL_D4 7

uint8_t numbers[10] = { 0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x27, 0x7F, 0x67};
uint8_t first_circle[10] = { 0x01, 0, 0, 0, 0, 0x08, 0x04, 0x02 };
uint8_t second_circle[10] = { 0, 0x01, 0x20, 0x10, 0x08, 0, 0, 0 };

uint32_t ms_count = 0;
uint32_t sec_count = 0;
uint32_t min_count = 0;

uint32_t stopwatch_ms_count = 0;
uint32_t stopwatch_sec_count = 0;

uint8_t dot_display = 0x80;

int is_stopwatch_on = 0;

#endif /* FND_H_ */