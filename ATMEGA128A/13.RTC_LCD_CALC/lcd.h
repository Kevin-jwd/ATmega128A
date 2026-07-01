/*
 * text_lcd.h
 *
 * Created: 2026-07-01 9:59:35
 *  Author: kccistc
 */ 
#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>

#define LCD_DATA_DDR	DDRB
#define LCD_DATA_PORT	PORTB
#define LCD_CONTROL_DDR		DDRC
#define LCD_CONTROL_PORT	PORTC

#define RS_PIN 0
#define RW_PIN 1
#define E_PIN 2

#define COMMAND_CLEAR_DISPLAY	0X01
#define COMMAND_4_BIT_MODE		0X28

#define COMMAND_DISPLAY_ON_OFF_BIT	2
#define COMMAND_CURSOR_ON_OFF_BIT	1
#define COMMAND_BLINK_ON_OFF_BIT	0