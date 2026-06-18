/*
 * washing_machine.h
 *
 * Created: 2026-06-18 오후 11:39:50
 *  Author: USER
 */ 


#ifndef WASHING_MACHINE_H_
#define WASHING_MACHINE_H_

// 0: 대기, 1: 세탁 시간 설정, 2: 헹굼 시간 설정, 3: 탈수 시간 설정, 4: 세탁, 5: 헹굼, 6: 탈수
typedef enum STATE { READY, TWASH, TRINSE, TDEHYDRATE, WASH, RINSE, DEHYDRATE};

extern volatile uint32_t state;
extern volatile uint32_t time_setting[3];

#endif /* WASHING_MACHINE_H_ */