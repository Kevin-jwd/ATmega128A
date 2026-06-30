/*
 * calc.h
 *
 * Created: 2026-06-29 오후 4:09:14
 *  Author: kccistc
 */ 


#ifndef CALC_H_
#define CALC_H_

#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>

void input_calculator(char key);

#endif /* CALC_H_ */