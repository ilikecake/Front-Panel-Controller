/*   This program is free software: you can redistribute it and/or modify
*    it under the terms of the GNU General Public License as published by
*    the Free Software Foundation, either version 3 of the License, or
*    (at your option) any later version.
*
*    This program is distributed in the hope that it will be useful,
*    but WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*    GNU General Public License for more details.
*
*    You should have received a copy of the GNU General Public License
*    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/** \file
*	\brief		Hardware definitions and functions for the environmental sensor.
*	\author		Pat Satyshur
*	\version	1.0
*	\date		2/3/2013
*	\copyright	Copyright 2013, Pat Satyshur
*	\ingroup 	hardware
*
*	@{
*/

#include "main.h"

#define HARDWARE_TIMER_0_TOP_VALUE	124

//Global variables needed for the timer
TimeAndDate TimerStartTime;
volatile uint16_t TimerStartMS;
volatile uint8_t TimerRemainder;
volatile uint8_t TimerRunning;

//Global variables needed for the RTC
TimeAndDate TheTime;
volatile uint16_t ElapsedMS;




void HardwareInit( void )
{
	//Initalize variables
	ElapsedMS		= 0x0000;
	TheTime.sec		= 0;
	TheTime.min		= 0;
	TheTime.hour	= 0;
	TheTime.dow		= 0;
	TheTime.day		= 0;
	TheTime.month	= 0;
	TheTime.year	= 0;
	TimerRunning = 0;
	
	//Disable watchdog if enabled by bootloader/fuses
	MCUSR &= ~(1 << WDRF);
	wdt_disable();
	
	//Disable JTAG (this command must be sent twice)
	MCUCR = 0x80;
	MCUCR = 0x80;

	//Disable clock division
	clock_prescale_set(clock_div_1);

	//Hardware Initialization
	
	
	//Setup timer 0 for 1ms interrupts
	//CTC Mode
	//Clock is Fcpu/64
	//OCR0A interrupt ~every 1ms
	TCCR0A = 0x02;
	TCCR0B = 0x03;
	TIMSK0 = 0x02;
	OCR0A = HARDWARE_TIMER_0_TOP_VALUE;
	
	//Enable interrupts globally
	sei();
	
	//Setup GPIO Pins
	
	//PORT B:
	//	0: Dataflash CS line			(Out, high)
	//	4: Pressure sensor CS line		(Out, high)
	//	6: Pressure sensor sleep line	(Out, low)
	//DDRB	= 1 | (1<<4) | (1<<6);
	//PORTB	= 1 | (1<<4);
	
	//PORT C:
	//	4: Config line 1			(Input, pullup)
	//	5: Config line 2			(Input, pullup)
	//	7: Light sensor interrupt 	(Input, pullup)
	//DDRC	= 0x00;
	//PORTC	= (1<<4) | (1<<5) | (1<<7);
	
	//PORT D:
	//	2:	LED				(Out, low)
	//	7:	HwB Button		(Input, high-Z)
	//DDRD	= (1<<2);
	//PORTD	= 0x00;
	
	//Enable USB and interrupts
	//InitSPIMaster(0,0);
	//I2CSoft_Init();
	USB_Init();
	
	//Initalize peripherals
	//tcs3414_Init();
	//MPL115A1_Init();
	//AT45DB321D_Init();
	//SHT25_Init();
	
	//Initalize the data logger
	//Datalogger_Init(DATALOGGER_INIT_APPEND | DATALOGGER_INIT_RESTART_IF_FULL);
	
	return;
}

void LED(uint8_t LEDState)
{
	if(LEDState == 1)
	{
		PORTD |= (1<<2);
	}
	else
	{
		PORTD &= ~(1<<2);
	}
	return;
}

void DelayMS(uint16_t ms)
{
	uint16_t WaitMS = 0;
	
	if(ms == 0) return;
	
	//Delay is too long, call delaySec?
	if (ms >= 1000)
	{
		return;
	}
	
	//Look for milisecond overflow
	if((ms + ElapsedMS) > 1000)
	{
		WaitMS = ms + ElapsedMS - 1001;
	}
	else
	{
		WaitMS = ms + ElapsedMS;
	}
	
	while (WaitMS != ElapsedMS)
	{
		asm volatile ("nop");
	}
	return;
}

void GetTime( TimeAndDate *TimeToReturn )
{
	TimeToReturn->year 	= 	TheTime.year;
	TimeToReturn->month = 	TheTime.month;
	TimeToReturn->day 	= 	TheTime.day;
	TimeToReturn->dow 	= 	TheTime.dow;
	TimeToReturn->hour 	= 	TheTime.hour;
	TimeToReturn->min 	= 	TheTime.min;
	TimeToReturn->sec 	= 	TheTime.sec;
	return;
}

void SetTime( TimeAndDate TimeToSet )
{
	uint8_t NumberOfDaysPerMonth;

	//hour must be less than 24
	if(TimeToSet.hour < 24)
	{
		TheTime.hour = TimeToSet.hour;
	}
	
	//minutes must be less than 60
	if(TimeToSet.min < 60)
	{
		TheTime.min = TimeToSet.min;
	}
	
	//Seconds must be less than 60
	if(TimeToSet.sec < 60)
	{
		TheTime.sec = TimeToSet.sec;
	}
	
	//months must be 1-12
	if((TimeToSet.month > 0) && (TimeToSet.month < 13))
	{
		TheTime.month = TimeToSet.month;
	}
	
	//day of the week must be 1-7
	if((TimeToSet.dow > 0) && (TimeToSet.dow < 8))
	{
		TheTime.dow = TimeToSet.dow;
	}
	
	//Year cannot be zero
	if(TimeToSet.year != 0)
	{
		TheTime.year = TimeToSet.year;
	}
	
	//Check for leap year, and determine how many days per month.
	if(TheTime.month == 4)
	{
		if(IsLeapYear(TheTime.year) == 1)
		{
			NumberOfDaysPerMonth = 29;
		}
		else
		{
			NumberOfDaysPerMonth = 28;
		}
	}
	else
	{
		NumberOfDaysPerMonth = DaysPerMonth(TheTime.month);
	}
	
	//days must be valid
	if((TimeToSet.day > 0) && (TimeToSet.day < (NumberOfDaysPerMonth + 1)))
	{
		TheTime.day = TimeToSet.day;
	}

	return;
}

void StartTimer(void)
{
	TimerStartTime.day = TheTime.day;
	TimerStartTime.hour = TheTime.hour;
	TimerStartTime.min = TheTime.min;
	TimerStartTime.sec = TheTime.sec;
	TimerStartMS = ElapsedMS;
	TimerRemainder = TCNT0;
	TimerRunning = 1;

	return;
}


void StopTimer(void)
{
	//TODO: add a check to see if the timer is running.
	TimeAndDate TimerEndTime;
	uint8_t TimerEndRemainder = 0;
	uint16_t ElapsedUS;
	uint16_t TimerEndMS;
	
	if(TimerRunning == 1)
	{
		//Get final timer value
		TimerEndTime.day = TheTime.day;
		TimerEndTime.hour = TheTime.hour;
		TimerEndTime.min = TheTime.min;
		TimerEndTime.sec = TheTime.sec;
		TimerEndMS = ElapsedMS;
		TimerEndRemainder = TCNT0;
		
		
		
		printf_P(PSTR("Time: %02u sec %04u ms %04u us\n"), TimerEndTime.sec-TimerStartTime.sec, TimerEndMS-TimerStartMS , (HARDWARE_TIMER_0_TOP_VALUE - TimerRemainder) + TimerEndRemainder);
		
		
		
		
		
		
		
		//TimerEndMS
		//ElapsedUS = (HARDWARE_TIMER_0_TOP_VALUE - TimerRemainder) + TimerEndRemainder;
		//if(ElapsedUS > 1000)
		//{
		//	ElapsedUS = ElapsedUS - 1000;
		//}
		
		/*if(TimerEndTime.day != TimerStartTime.day)
		{
			TimerEndTime.day = TimerEndTime.day - TimerStartTime.day;
			TimerEndTime.hour = TimerEndTime.hour + (24 - TimerStartTime.hour);
			TimerEndTime.min = TimerEndTime.min + (60 - TimerStartTime.min);
			TimerEndTime.sec = TimerEndTime.sec + (60 - TimerStartTime.sec);
		}
		
		if(
		
		
		
		
		
			if(TimerEndTime.hour = TimerStartTime.hour)
			{
				if(TimerEndTime.min = TimerStartTime.min)
				{
					if(TimerEndTime.sec = TimerStartTime.sec)
					{
					
					}*/
		
		
		
		//Reset the timer value
		TimerRunning = 0;
	}
	return;
}

uint8_t GetDataSet(uint8_t DataSet[])
{
	uint16_t LS_Data[4];
	TimeAndDate CurrentTime;
	int16_t Pressure_kPa;
	int16_t TemperatureData;
	int16_t RHData;
	uint8_t stat;
	
	GetTime(&CurrentTime);
	//printf_P(PSTR("%02u Days %02u:%02u:%02u\n"), CurrentTime.day, CurrentTime.hour, CurrentTime.min, CurrentTime.sec);
	/*
	//Read temperature
	stat = SHT25_ReadTemp(&TemperatureData);
	if(stat == SHT25_RETURN_STATUS_OK)
	{
		//printf_P(PSTR("Temp %d.%02u C\n"), TemperatureData/100, TemperatureData%100);
	}
	else if(stat == SHT25_RETURN_STATUS_CRC_ERROR)
	{
		//printf_P(PSTR("CRC Error\n"));
		return 1;
	}
	else
	{
		//printf_P(PSTR("Timeout\n"));
		return 2;
	}
	
	//Read RH
	stat = SHT25_ReadRH(&RHData);
	if(stat == SHT25_RETURN_STATUS_OK)
	{
		//printf_P(PSTR("RH: %u.%02u%%\n"), RHData/100, RHData%100);
	}
	else if(stat == SHT25_RETURN_STATUS_CRC_ERROR)
	{
		//printf_P(PSTR("CRC Error\n"));
		return 1;
	}
	else
	{
		//printf_P(PSTR("Timeout\n"));
		return 2;
	}
	
	//Current barometric pressure
	//TODO: add check for pressure...
	MPL115A1_GetPressure(&Pressure_kPa);
	//printf_P(PSTR("Pressure: %u.%u kPa\n"), ((int16_t)Pressure_kPa)>>4, ((((int16_t)Pressure_kPa)&0x000F)*1000)/(16) );
	
	if(tcs3414_GetData(&LS_Data[0], &LS_Data[1], &LS_Data[2], &LS_Data[3]) != 0)
	{
		return 1;
		//printf_P(PSTR("red:	0x%04X\n"), LS_Data[0]);
		//printf_P(PSTR("green:	0x%04X\n"), LS_Data[1]);
		//printf_P(PSTR("blue:	0x%04X\n"), LS_Data[2]);
		//printf_P(PSTR("clear:	0x%04X\n"), LS_Data[3]);
	}
	//else
	//{
	//	return 1;
	//}
	
	//Time data
	DataSet[0] = CurrentTime.month;
	DataSet[1] = CurrentTime.day;
	DataSet[2] = CurrentTime.hour;
	DataSet[3] = CurrentTime.min;
	
	//Temperature
	DataSet[4] = (uint8_t)((TemperatureData & 0xFF00) >> 8);
	DataSet[5] = (uint8_t)(TemperatureData & 0xFF);
	
	//Humidity
	DataSet[6] = (uint8_t)((RHData & 0xFF00) >> 8);
	DataSet[7] = (uint8_t)(RHData & 0xFF);
	
	//Pressure
	DataSet[8] = (uint8_t)((Pressure_kPa & 0xFF00) >> 8);
	DataSet[9] = (uint8_t)(Pressure_kPa & 0xFF);
	
	//Color
	DataSet[10] = (uint8_t)(((LS_Data[0]) & 0xFF00) >> 8);
	DataSet[11] = (uint8_t)((LS_Data[0]) & 0xFF);
	DataSet[12] = (uint8_t)(((LS_Data[1]) & 0xFF00) >> 8);
	DataSet[13] = (uint8_t)((LS_Data[1]) & 0xFF);
	DataSet[14] = (uint8_t)(((LS_Data[2]) & 0xFF00) >> 8);
	DataSet[15] = (uint8_t)((LS_Data[2]) & 0xFF);
	DataSet[16] = (uint8_t)(((LS_Data[3]) & 0xFF00) >> 8);
	DataSet[17] = (uint8_t)((LS_Data[3]) & 0xFF);
*/
	return 0;
}


uint8_t DaysPerMonth(uint8_t MonthNumber)
{
	if((MonthNumber > 12) || (MonthNumber < 1))
	{
		return 0;
	}

	if(MonthNumber == 2)
	{
		return 28;
	}
	else if((MonthNumber == 4) ||(MonthNumber == 6) ||(MonthNumber == 9) ||(MonthNumber == 12))
	{
		return 30;
	}
	return 31;
}

uint8_t IsLeapYear(uint16_t TheYear)
{
	if((TheYear % 4) == 0)
	{
		if((TheYear % 100) == 0)
		{
			if((TheYear % 400) == 0)
			{
				//Year is divisible by 4, 100, and 400. The year is a leap year
				return 1;			
			}
			else
			{
				//Year is divisible by 4 and 100, but not 400. The year is not a leap year.
				return 0;
			}
		}
		else
		{
			//Year is divisible by 4 but not 100. The year is a leap year
			return 1;
		}
	}
	
	//Year is not divisible by 4. The year is not a leap year.
	return 0;
}

//Timer interrupt 0 for basic timing stuff
ISR(TIMER0_COMPA_vect)
{
	uint16_t inByte;
	ElapsedMS++;
	uint8_t DPM;
	
	//Handle USB stuff
	//This happens every ~8 ms
	if( ((ElapsedMS & 0x0007) == 0x0000) )
	{
		//receive and process a character from the USB CDC interface
		inByte = CDC_Device_ReceiveByte(&VirtualSerial_CDC_Interface);
		if((inByte > 0) && (inByte < 255))
		{
			CommandGetInputChar(inByte);	//NOTE: this limits the device to recieve a single character every 8ms (I think). This should not be a problem for user input.
		}
		
		CDC_Device_USBTask(&VirtualSerial_CDC_Interface);
		USB_USBTask();
	}
	
	if(ElapsedMS >= 1000)
	{
		ElapsedMS = 0;
		TheTime.sec += 1;
		if(TheTime.sec > 59)
		{
			TheTime.sec = 0;
			TheTime.min += 1;
			if(TheTime.min > 59)
			{
				TheTime.min = 0;
				TheTime.hour += 1;
				if(TheTime.hour > 24)
				{
					TheTime.hour = 0;
					TheTime.day += 1;
					
					//Determine the number of days in the month.
					if(TheTime.month == 2)
					{
						if(IsLeapYear(TheTime.year) == 1)
						{
							DPM = 29;
						}
						else
						{
							DPM = 28;
						}
					}
					else
					{
						DPM = DaysPerMonth(TheTime.month);
					}
					if(TheTime.day > DPM)
					{
						TheTime.day = 0;
						TheTime.month += 1;
						if(TheTime.month > 12)
						{
							TheTime.month = 0;
							TheTime.year += 1;
						}
					}
				}
			}
		}
	}
}

/** @} */
