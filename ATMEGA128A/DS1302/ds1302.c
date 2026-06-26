/*
 * ds1302.c
 *
 * Created: 2026-06-26 오후 2:42:30
 *  Author: kccistc
 */ 
#include "ds1302.h"

uint8_t dtob(uint8_t data);
uint8_t btod(uint8_t data);
void init_ds1302_date_time(void);
void init_ds1302_ddr(void);
void init_ds1302_gpio(void);
void init_ds1302(void);
void clock_ds1302(void);
void write_ds1302(uint8_t address, uint8_t data);
void read_ds1302_time(uint8_t address);
uint8_t read_ds1302(uint8_t address);
void rx_ds1302(uint8_t *pdata);
void transmit_ds1302(uint8_t data);

t_ds1302 ds1302;

/* DS1302 main */
void ds1302_main()
{
	init_ds1302_date_time();
	init_ds1302_ddr();
	init_ds1302_gpio();
	init_ds1302();
	
	while(1)
	{
		// 1. read time
		read_time_ds1302();
		// 2. read date
		
		// 3. printf (date, time)
		printf("***%4d-%2d-%2d %2d:%2d:%2d");
		
		// 4. _delay_ms(1000)
		_delay_ms(1000);
	}
}

/* Decimal to DCB */
uint8_t dtob(uint8_t data)
{
	uint8_t high, low;         // high nibble, low nibble
	high = (data / 10) << 4;   
	low  = (data % 10);
	
	return high + low;
}

/* DCB to Decimal */
uint8_t btod(uint8_t data)
{
	uint8_t high, low;
	
	high = (data >> 4) * 10;
	low = data & 0x0F;
	return high + low;
}

/* Initialize DS1302 struct member variables */
// Reason why DS1302 should be initialized is that there are trash values at first if it is not.
void init_ds1302_date_time()
{
	ds1302.seconds   = 00;
	ds1302.minutes   = 20;
	ds1302.hours     = 15;
	ds1302.date      = 26;
	ds1302.day       =  6;           
	ds1302.year      = 26;
	ds1302.ampm      =  1;
	ds1302.hour_mode =  0;
}

/* Initialize DS1302 DDR */
void init_ds1302_ddr()
{
	DS1302_DDR &= ~(1 << DS1302_CLK | 1 << DS1302_DAT | 1 << DS1302_RST);    // reset CLK, DAT, RST
	DS1302_DDR |= 1 << DS1302_CLK | 1 << DS1302_DAT | 1 << DS1302_RST;       // set   CLK, DAT, RST
}

/* Initialize DS1302 GPIO pins */
// Reset all 3 pins (GPIO CE, SCK, I/O) at first to make signals low.
void init_ds1302_gpio()
{
	DS1302_DDR &= ~(1 << DS1302_CLK | 1 << DS1302_DAT | 1 << DS1302_RST);    // reset CLK, DAT, RST
	_delay_ms(2);                                                            // to make signal stable
}

/* Initialize DS1302 */
void init_ds1302()
{
	write_ds1302(SEC_WRITE_ADDRESS, ds1302.seconds);
	write_ds1302(MIN_WRITE_ADDRESS, ds1302.minutes);
	write_ds1302(HOUR_WRITE_ADDRESS, ds1302.hours);
	write_ds1302(DATE_WRITE_ADDRESS, ds1302.date);
	write_ds1302(MONTH_WRITE_ADDRESS, ds1302.month);
	write_ds1302(DAY_WRITE_ADDRESS, ds1302.day);	
	write_ds1302(YEAR_WRITE_ADDRESS, ds1302.year);
}

/* Generate CLK signal (one pulse) */
void clock_ds1302()
{
	DS1302_PORT &= ~(1 << DS1302_CLK);    // LOW
	DS1302_PORT |=    1 << DS1302_CLK;    // HIGH
	DS1302_PORT &= ~(1 << DS1302_CLK);    // LOW
}

/* Single Byte Write Operation */
void write_ds1302(uint8_t address, uint8_t data)
{
	// 1. Pull-up CE(Chip Enable) Low to High
	DS1302_PORT |= 1 << DS1302_RST;
	
	// 2. Transmit address
	transmit_ds1302(address);
	
	// 3. Transmit data
	transmit_ds1302(dtob(data));
	
	// 4. Pull-down CE High to Low
	DS1302_PORT &= ~(1 << DS1302_RST);
}

/* Single Byte Read Operation */
void read_time_ds1302()
{
	ds1302.seconds = read_ds1302(SEC_WRITE_ADDRESS);
	ds1302.minutes = read_ds1302(MIN_READ_ADDRESS);
	ds1302.hours = read_ds1302(HOUR_READ_ADDRESS);
}

void read_date_ds1302()
{
	ds1302.date = read_ds1302(DATE_READ_ADDRESS);
	ds1302.month = read_ds1302(MONTH_READ_ADDRESS);
	ds1302.day = read_ds1302(DAY_READ_ADDRESS);
	ds1302.year = read_ds1302(YEAR_READ_ADDRESS);
}

uint8_t read_ds1302(uint8_t address)
{
	uint8_t data = 0;       // 8bits; 1bit씩 읽어서 담을 변수
	// 1. Pull-up CE Low to High 
	DS1302_PORT |= 1 << DS1302_RST;
	// 2. Transmit address
	transmit_ds1302(address + 1);
	// 3. Read data
	rx_ds1302(&data);
	// 4. Pull-up CE Low to High 
	DS1302_PORT &= ~(1 << DS1302_RST);
	// 5. return (bcd to dec)
	return btod(data);
}

void rx_ds1302(uint8_t *pdata)
{
	uint8_t temp = 0;
	DS1302_DDR &= ~(1 << DS1302_DAT);        // set DS1302 input mode
	
	// Input data from LSB in order
	for(int i = 0; i < 8;i++)
	{
		if(DS1302_DAT_PIN & (1 << DS1302_DAT))
		{
			temp |= 1 << i;           // set only if 1
		}
		if(i != 7) clock_ds1302();
	}
	*pdata = temp;
}

/* Transmit DS1302 data */
void transmit_ds1302(uint8_t data)
{
	DS1302_DDR |= 1 << DS1302_DAT;              // set DS1302 output mode
	for(int i = 0; i < 8;i++)
	{
		if(data & (1 << i))
		{
			DS1302_PORT |= 1 << DS1302_DAT;       // 1
		}
		else
		{
			DS1302_PORT &= ~(1 << DS1302_DAT);    // 0
		}
		clock_ds1302();                          
	}
}


