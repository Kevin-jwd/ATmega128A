/*
 * ds1302.c
 *
 * Created: 2026-06-26 오후 2:42:14
 *  Author: user
 */ 

#include "ds1302.h"

extern void init_uart0(void);
extern void pc_command_processing(t_ds1302 *sp);

void init_date_time(t_ds1302 *sp);
void init_ddr_ds1302(void);
void init_gpio_ds1302(void);
void init_ds1302(t_ds1302 *sp);
void ds1302_main(void);
void read_time_ds1302(t_ds1302 *sp);
void read_date_ds1302(t_ds1302 *sp);

uint8_t bcd2dec(uint8_t data); 
uint8_t dec2bcd(uint8_t data);  //
uint8_t read_ds1302(uint8_t addr);  //
 
void write_ds1302(uint8_t addr, uint8_t data);
void tx_ds1302(uint8_t data);
void clock_ds1302(void);


void ds1302_main(void)
{
	t_ds1302  ds1302;  
	t_ds1302 *sp = &ds1302;
	
	init_date_time(sp);
	init_ddr_ds1302();
	init_gpio_ds1302();  // all LOW로 설정 
	// init_ds1302(sp);
	
	while (1)
	{
		pc_command_processing(sp);
		// 1. read time
		read_time_ds1302(sp);
		// 2. read date
		read_date_ds1302(sp);
		// 3. printf date & time
		printf("%d.%d.%d\t%d:%d:%d\n", sp->year, sp->month, sp->date, sp->hours, sp->minutes, sp->seconds);
		// 4. delay_ms(1000);
	   _delay_ms(1000);
	}
}

void read_time_ds1302(t_ds1302 *sp)
{
	sp->seconds = read_ds1302(ADDR_SECONDS);
	sp->minutes = read_ds1302(ADDR_MINUTES);
	sp->hours = read_ds1302(ADDR_HOUR);
}

void read_date_ds1302(t_ds1302 *sp)
{
	sp->year = read_ds1302(ADDR_YEAR);
	sp->month = read_ds1302(ADDR_MONTH);
	sp->date = read_ds1302(ADDR_DATE);
}

uint8_t read_ds1302(uint8_t addr)
{
	uint8_t data8bits=0;  // 1bit씩 읽어서 담을 변수 
	// 4. CE low
	// 5. return (bcd to dec)
	// 1. CE low--> HIGH
	DS1302_RST_PORT |= 1 << DS1302_RST;
	// 2. ADDR 전송
	tx_ds1302(addr+1);  // read addr
	// 3. DATA 읽어들인다. 
	rx_ds1302(&data8bits);
	// 4. CE HIGH --> LOW
	DS1302_RST_PORT &= ~(1 << DS1302_RST);
	
	return(bcd2dec(data8bits));
}

void rx_ds1302(uint8_t *pdata8bits)
{
	uint8_t temp=0;
	
	//1. 입력 mode로 설정
	DS1302_DAT_DDR &= ~(1 << DS1302_DAT);  // read mode
    
	// LSB로부터 차례로 입력
	for (int i=0; i < 8; i++)
	{
		if (DS1302_DAT_PIN & (1 << DS1302_DAT))
		{
			temp |= 1 << i;   // 1의 조건만 set
		}
		// 3. CLK를 HIGH--> LOW
		if (i != 7)  // 마지막 bit를 읽을 때는 clk 을 보내주지 않는다. 
			clock_ds1302();
	}
	
	*pdata8bits = temp;
}

uint8_t *burst_mode_read_ds1302(uint8_t data)
{
	static uint8_t time_date_register[8] ={ 0 };
	
	// 1. CE low --> high
	DS1302_RST_PORT |= 1 << DS1302_RST;
	
	// 2. send addr (read: 0xBF)
	tx_ds1302(0xBF);

	// --7----6-----5---4---3---2---1-----0--
	// --1--RAM/CK--A4--A3--A2--A1--A0--RD/WR
	// 1  0       0   0   0   0   0   1     : 0x81
	rx_ds1302(&time_date_register);
	
	//1. 입력 mode로 설정
	DS1302_DAT_DDR &= ~(1 << DS1302_DAT);  // read mode
	
	// LSB로부터 차례로 입력
	for (int i=0; i < 8; i++)
	{
		if (DS1302_DAT_PIN & (1 << DS1302_DAT))
		{
			time_date_register[i] |= 1 << i;   // 1의 조건만 set
		}
		// 3. CLK를 HIGH--> LOW
		if (i != 7)  // 마지막 bit를 읽을 때는 clk 을 보내주지 않는다.
		clock_ds1302();
	}
	
	// 4. CE HIGH --> LOW
	DS1302_RST_PORT &= ~(1 << DS1302_RST);
	return time_date_register;
}

void init_ds1302(t_ds1302 *sp)
{
	write_ds1302(ADDR_SECONDS, sp->seconds);
	write_ds1302(ADDR_MINUTES, sp->minutes);
	write_ds1302(ADDR_HOUR, sp->hours);
	write_ds1302(ADDR_DATE, sp->date);
	write_ds1302(ADDR_MONTH, sp->month);
	write_ds1302(ADDR_DAYOFWEEK, sp->dayofweek);
	write_ds1302(ADDR_YEAR, sp->year);
}
// dec --> bcd
// 예) 25
//   dec        bcd
// 00011001   0010 0101
uint8_t dec2bcd(uint8_t data)
{
	uint8_t high, low;
	
	high = (data /10) << 4;  // high nibble에 위치
	low = data % 10; 
	
	return(high+low); 
}

// 1. 입력 bcd
// 예) 26년의 bcd 
//  7654 3210
//  0010 0110
//    2    6
//  x10   x1
//============
//    26
uint8_t bcd2dec(uint8_t data)
{
	uint8_t high, low;
	
	low = data & 0x0f;
	high = (data >> 4) * 10;
	
	return(high+low);
}

void write_ds1302(uint8_t addr, uint8_t data)
{
	// 1. CE low--> HIGH
	DS1302_RST_PORT |= 1 << DS1302_RST;
	// 2. ADDR 전송
	tx_ds1302(addr);
	// 3. DATA 전송
	tx_ds1302(dec2bcd(data));
	// 4. CE HIGH --> LOW 
	DS1302_RST_PORT &= ~(1 << DS1302_RST);
}

void tx_ds1302(uint8_t data)
{
	//1. 출력 mode로 설정
	DS1302_DAT_DDR |= 1 << DS1302_DAT;  // write mode 
	// 예) 0x80
	//  M      L
	//  1000 000
	for (int i=0; i < 8; i++)
	{
		// 2. 1 bit를 전송
		if (data & (1 << i))
			DS1302_DAT_PORT |= 1 << DS1302_DAT;   // 1
		else DS1302_DAT_PORT &= ~(1 << DS1302_DAT); // 0
		// 3. CLK를 HIGH--> LOW
		clock_ds1302();	
	}
}

void clock_ds1302(void)
{
	// LOW --> HIGH --> LOW 
	DS1302_CLK_PORT &= ~(1 << DS1302_CLK);
	DS1302_CLK_PORT |= 1 << DS1302_CLK;	
	DS1302_CLK_PORT &= ~(1 << DS1302_CLK);
}
void init_ddr_ds1302(void)
{
	DDRF &= ~(1 << DS1302_CLK | 1 << DS1302_DAT | 1 << DS1302_RST);
	DDRF |= 1 << DS1302_CLK | 1 << DS1302_DAT | 1 << DS1302_RST;  // 출력 mode로 설정 
}

void init_gpio_ds1302(void)
{
	DS1302_CLK_PORT &= ~(1 << DS1302_CLK | 1 << DS1302_DAT | 1 << DS1302_RST);
	_delay_ms(2); 
}

void init_date_time(t_ds1302 *sp)
{
	sp->year=26;
	sp->month=06;
	sp->date=26;
	sp->dayofweek=6;   // Fri
	sp->hours=9;
	sp->minutes=26;
	sp->seconds=33;
}