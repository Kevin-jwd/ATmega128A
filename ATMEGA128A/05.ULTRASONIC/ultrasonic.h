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

#define TRIGGER_DDR DDRA
#define TRIGGER_PORT PORTA
#define TRIGGER_PIN 2

#define ECHO_DDR DDRE
#define ECHO_PORT PINE
#define ECHO_PIN 6

#endif /* ULTRASONIC_H_ */