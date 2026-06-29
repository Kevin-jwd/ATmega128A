/*
 * ds1302.c
 *
 * Created: 2026-06-26 오후 2:42:14
 *  Author: user
 */ 

#include "ds1302.h"

void init_date_time(void);
void init_ddr_ds1302(void);
void init_gpio_ds1302(void);
void init_ds1302(void);
void ds1302_main(void);

uint8_t bcd2dec(uint8_t data); 
uint8_t dec2bcd(uint8_t data);  //
uint8_t read_ds1302(uint8_t addr);  //
 
void write_ds1302(uint8_t addr, uint8_t data);
void tx_ds1302(uint8_t data);
void clock_ds1302(void);


void ds1302_main(void)
{
	init_date_time();
	init_ddr_ds1302();
	init_gpio_ds1302();  // all LOW로 설정 
	// init_ds1302();
	
	while (1)
	{
		// 1. read time
		read_time_ds1302();
		// 2. read date
		// 3. printf date & time
		printf("%d-%d-%d\n", ds1302.hours,ds1302.minutes,ds1302.seconds);
		// 4. delay_ms(1000);
	   _delay_ms(1000);
	}
}

void read_time_ds1302(void)
{
	ds1302.seconds = read_ds1302(ADDR_SECONDS);
	ds1302.minutes = read_ds1302(ADDR_MINUTES);
	ds1302.hours = read_ds1302(ADDR_HOUR);
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
	
void init_ds1302(void)
{
	write_ds1302(ADDR_SECONDS, ds1302.seconds);
	write_ds1302(ADDR_MINUTES, ds1302.minutes);
	write_ds1302(ADDR_HOUR, ds1302.hours);
	write_ds1302(ADDR_DATE, ds1302.date);
	write_ds1302(ADDR_MONTH, ds1302.month);
	write_ds1302(ADDR_DAYOFWEEK, ds1302.dayofweek);
	write_ds1302(ADDR_YEAR, ds1302.year);
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

void init_date_time(void)
{
	ds1302.year=26;
	ds1302.month=06;
	ds1302.date=26;
	ds1302.dayofweek=6;   // Fri
	ds1302.hours=9;
	ds1302.minutes=26;
	ds1302.seconds=33;
}