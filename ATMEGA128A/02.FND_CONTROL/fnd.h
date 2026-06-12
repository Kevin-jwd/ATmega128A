/*
 * fnd.h
 *
 * Created: 2026-06-12 오전 10:46:02
 *  Author: kccistc
 */ 


#ifndef FND_H_
#define FND_H_

#define F_CPU 16000000UL   
#include <avr/io.h>
#include <util/delay.h>

#define FND_DATA_DDR DDRC
#define FND_DATA_PORT PORTC

#define FND_CONTROL_DDR DDRB
#define FND_CONTROL_PORT PORTB

#define FND_CONTROL_D1 4
#define FND_CONTROL_D2 5
#define FND_CONTROL_D3 6
#define FND_CONTROL_D4 7

#endif /* FND_H_ */