/*
 * button.h
 *
 * Created: 2026-06-10 오전 11:45:05
 *  Author: kccistc
 */ 
#ifndef  BUTTON_H_
#define BUTTON_H_
#include <avr/io.h>
#include <util/delay.h>        

#define BUTTON_DDR DDRD
#define BUTTON_PIN PIND          // PORTD를 읽는 register (5V; 1, 0V; 0)

#define BUTTON0PIN 3             // PORTD.3
#define BUTTON1PIN 4             // PORTD.4
#define BUTTON2PIN 5             // PORTD.5
#define BUTTON3PIN 6             // PORTD.6

#define BUTTON0 0                // PORTD.3의 가상 index
#define BUTTON1 1                // PORTD.4의 가상 index
#define BUTTON2 2                // PORTD.5의 가상 index
#define BUTTON3 3                // PORTD.6의 가상 index

#define BUTTON_NUMBER 4          // 버튼 갯수

#define BUTTON_PRESS  1          // 버튼 누르면         HIGH (Active High)
#define BUTTON_RELEASE 0         // 버튼 누르지 않으면   LOW 

#endif
