/*
 * 08.PWM_SERVO_PIEZO_BUZZER.c
 *
 * Created: 2026-06-25 오전 9:46:12
 * Author : kccistc
 */ 

// ====== SERVO MOTOR ======
#if 0         
#include <avr/io.h>

extern init_sp

int servo_motor_main(void);

int main(void)
{
	servo_motor_main();
    while (1) 
    {
		;
    }
}
#endif

// ====== PIEZO BUZZER ======
#if 1
#define F_CPU	16000000UL
#include <avr/io.h>
#include <util/delay.h>

#include "button.h"

extern void Music_Player(int *tone, int *Beats);
extern void init_speaker(void);
extern void Beep(int  repeat);
extern void Siren(int repeat);
extern void RRR(void);
extern void power_on_melody();
extern void open_buzzer();
extern void drum_washing_stop_melody();

extern const int He_Pirate[];
extern const int He_Pirate1_Beat[];
extern const int Elise_Tune[];
extern const int Elise_Beats[];
extern const int Polka_Tune[];
extern const int Polka_Beats[];

typedef enum STATE { POWER_ON_MELODY, OPEN_BUZZER, DRUM_WASHING_STOP_MELODY };
void (*func[]) () = { power_on_melody, open_buzzer, drum_washing_stop_melody };
extern const int LG_End_Tune[];
extern const int LG_End_Beats[];

uint8_t button_state = -1;
uint8_t is_stop = 0;

// PE3 (OC3A) PWM 출력 사용.
// 16bit Timer/Counter
// OCR3A값이 같아지면 Low 출력.

int main(void)
{
	init_speaker();	

	while(1)
	{
		if(get_button(BUTTON0, BUTTON0PIN))
		{
			button_state = POWER_ON_MELODY;
		}
		else if(get_button(BUTTON1, BUTTON1PIN))
		{
			button_state = OPEN_BUZZER;
		}
		else if(get_button(BUTTON2, BUTTON2PIN))
		{
			button_state = DRUM_WASHING_STOP_MELODY;
		}
		
		switch(button_state)
		{
			case POWER_ON_MELODY:
				power_on_melody();
				if(get_button(BUTTON0, BUTTON0PIN)) 
				{
					is_stop = ~is_stop;
				}
				break;
			case OPEN_BUZZER:
				open_buzzer();
				if(get_button(BUTTON1, BUTTON1PIN)) 
				{
					is_stop = ~is_stop;
				}
				break;
			case DRUM_WASHING_STOP_MELODY:
				drum_washing_stop_melody();
				if(get_button(BUTTON2, BUTTON2PIN)) 
				{
					is_stop = ~is_stop;
				}
				break;
			default:
				break;
		}
	}
}

#endif
