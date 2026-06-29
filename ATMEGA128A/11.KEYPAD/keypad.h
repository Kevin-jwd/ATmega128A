/*
 * keypad.h
 *
 * Created: 2026-06-29 오후 2:11:11
 *  Author: kccistc
 */ 


#ifndef KEYPAD_H_
#define KEYPAD_H_

#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
/* Common */
#define ROW_LENGTH    4
#define COLUMN_LENGTH 4

#define KEYPAD_DDR  DDRA
#define KEYPAD_PIN  PINA
#define KEYPAD_PORT PORTA

/* Row */
#define KEYPAD_R1_PORT_NUM 7
#define KEYPAD_R2_PORT_NUM 6
#define KEYPAD_R3_PORT_NUM 5
#define KEYPAD_R4_PORT_NUM 4

/* Column */
#define KEYPAD_C1_PORT_NUM 3
#define KEYPAD_C2_PORT_NUM 2
#define KEYPAD_C3_PORT_NUM 1
#define KEYPAD_C4_PORT_NUM 0

#endif /* KEYPAD_H_ */