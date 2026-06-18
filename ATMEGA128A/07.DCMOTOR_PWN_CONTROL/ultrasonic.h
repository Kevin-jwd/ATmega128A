/*
 * ultrasonic.h
 *
 * Created: 2026-06-17 오후 1:47:56
 *  Author: kccistc
 */ 


#ifndef ULTRASONIC_H_
#define ULTRASONIC_H_

#define F_CPU 16000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#define TRIGGER_DDR DDRG
#define TRIGGER_PORT PORTG
#define TRIGGER_PIN 4

#define ECHO_DDR DDRE
#define ECHO_PORT PINE
#define ECHO_PIN 4

#endif /* ULTRASONIC_H_ */