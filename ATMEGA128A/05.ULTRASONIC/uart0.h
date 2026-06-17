/*
 * uart0.h
 *
 * Created: 2026-06-16 오전 9:57:36
 *  Author: kccistc
 */ 


#ifndef UART0_H_
#define UART0_H_

#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <string.h>

#define QUEUE_SIZE 10
#define COMMAND_LENGTH 80

volatile int rear = 0;
volatile int front = 0;

volatile unsigned char rx_buff[QUEUE_SIZE][COMMAND_LENGTH];

#endif /* UART0_H_ */