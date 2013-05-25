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
#include "MicroMenu.h"

#define HARDWARE_TIMER_0_TOP_VALUE	124

#define BUTTON_DELAY_COUNT	2

//Global variables needed for the timer
TimeAndDate TimerStartTime;
volatile uint16_t TimerStartMS;
volatile uint8_t TimerRemainder;
volatile uint8_t TimerRunning;

//Global variables needed for the RTC
TimeAndDate TheTime;
volatile uint16_t ElapsedMS;

volatile uint8_t OutputTimeToLCD;

volatile uint8_t ButtonInputTimeoutCount;







////////////////////////////////////

/** Example menu item specific enter callback function, run when the associated menu item is entered. */
static void Level1Item1_Enter(void)
{
	lcd_puts("ENTER");
}

/** Example menu item specific select callback function, run when the associated menu item is selected. */
static void Level1Item1_Select(void)
{
	lcd_puts("SELECT");
}

/** Generic function to write the text of a menu.
 *
 *  \param[in] Text   Text of the selected menu to write, in \ref MENU_ITEM_STORAGE memory space
 */
static void Generic_Write(const char* Text)
{
	if (Text)
	{
		lcd_clrscr();
		lcd_puts_p(Text);
	}
}

//MENU_ITEM(Name, Next, Previous, Parent, Child, SelectFunc, EnterFunc, Text)
MENU_ITEM(Menu_1, Menu_2, Menu_3, NULL_MENU, Menu_1_1,  NULL, NULL, "Menu\nItem 1");
MENU_ITEM(Menu_2, Menu_3, Menu_1, NULL_MENU, NULL_MENU, NULL, NULL, "Menu\nItem 2");
MENU_ITEM(Menu_3, Menu_1, Menu_2, NULL_MENU, NULL_MENU, NULL, Jump_To_Bootloader, "Menu\nDFU Mode");

MENU_ITEM(Menu_1_1, Menu_1_2, Menu_1_2, Menu_1, NULL_MENU, NULL, NULL, "1.1");
MENU_ITEM(Menu_1_2, Menu_1_1, Menu_1_1, Menu_1, NULL_MENU, NULL, NULL, "Jon is funny\n  looking!");

///////////////////////////////////////////


void HandleButtonPress(uint8_t Button);













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
	
	ButtonInputTimeoutCount = 0;
	OutputTimeToLCD = 0;
	
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
	
	
	//Setup timer 1 for switch debouncing
	//Normal Mode
	//Clock is Fcpu/1024
	//OCR0A interrupt ~every 250ms
	TCCR1A = 0x00;
	TCCR1B = 0x00;
	TCNT1H = 0x00;		//clear the timer
	TCNT1L = 0x00;
	TIMSK1 = 0x03;
	OCR1AH = 0x07;
	OCR1AL = 0xA1;
	
	//Enable interrupts globally
	sei();
	
	//Setup GPIO Pins
	
	//PORT B:
	//	0: Dataflash CS line			(Out, high)
	//	4: Pressure sensor CS line		(Out, high)
	//	6: Backlight control			(Out, high)
	DDRB	= (1<<6);
	PORTB	= (1<<6);
	
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

	USB_Init();
	
	//Initalize LCD
	lcd_init(LCD_DISP_ON);
	
	//clear display and home cursor
	lcd_clrscr();
	
	EnableButtons();
	
	/* Set up the default menu text write callback, and navigate to an absolute menu item entry. */
	Menu_SetGenericWriteCallback(Generic_Write);
	//Menu_Navigate(&Menu_1);
	
	OutputTimeToLCD = 1;
	
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

void EnableButtons(void)
{
	DDRC &= 0xFB;		//PC2 is set as input
	PORTC |= 0x04;		//Enable pullup on PC2
	
	DDRD &= 0xCC;		//PD0, PD1, PD4, and PD5 are set as input
	PORTD |= 0x33;		//Enable pullups on PD0, PD1, PD4, and PD5
	
	EICRA = 0x0A;		//INT0 and INT1 trigger on falling edge
	EICRB = 0x08;		//INT5 trigger on falling edge
	
	EIFR = 0xFF;		//Clear all interrupts
	EIMSK = 0x23;		//Enable INT0, INT1, INT5
	
	PCMSK1 = 0x18;		//Unmask PCINT11 and PCINT12
	PCIFR = 0x03;		//Clear pin change interrupts
	PCICR = 0x02;		//Enable PCINT1
	
	return;
}

void DisableButtons(void)
{
	EIMSK = 0x00;		//Disable interrupts
	PCICR = 0x00;		//Disable pin change interrupts
	
	EIFR = 0xFF;		//Clear all interrupts
	PCIFR = 0x03;		//Clear pin change interrupts
	return;
}

void StartDebounceTimer(void)
{
	ButtonInputTimeoutCount = 0;
	TCCR1B &= 0xF8;		//Stop the timer (if it is running)
	TCNT1H = 0x00;		//clear the timer
	TCNT1L = 0x00;
	TCCR1B |= 0x05;		//Start timer 1 for debouncing
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

/*Button:
 * 1 - left
 * 2 - right
 * 3 - up
 * 4 - down
 * 5 - center
 */
void HandleButtonPress(uint8_t Button)
{
	DisableButtons();
	StartDebounceTimer();
	
	if(OutputTimeToLCD == 1)
	{
		OutputTimeToLCD = 0;
		lcd_clrscr();
		Menu_Navigate(&Menu_1);
	}
	else
	{
		if(Button == 1)
		{
			Menu_Navigate(MENU_PARENT);
		}
		else if(Button == 2)
		{
			Menu_Navigate(MENU_CHILD);
		}
		else if(Button == 3)
		{
			Menu_Navigate(MENU_PREVIOUS);
		}	
		else if(Button == 4)
		{
			Menu_Navigate(MENU_NEXT);
		}
		else if(Button == 5)
		{
			Menu_EnterCurrentItem();
		}
	}
	
	return;
}


ISR(INT0_vect)
{
	HandleButtonPress(1);	//Left

	/*
	if(OutputTimeToLCD == 1)
	{
		OutputTimeToLCD = 0;
		lcd_clrscr();
		Menu_Navigate(&Menu_1);
	}
	else
	{
		DisableButtons();
		StartDebounceTimer();
		
		//lcd_clrscr();
		//lcd_puts("Left\n");
		printf_P(PSTR("Left\n"));
		Menu_Navigate(MENU_PARENT);
	}*/
}

ISR(INT1_vect)
{
	HandleButtonPress(3);	//Up
	/*
	if(OutputTimeToLCD == 1)
	{
		OutputTimeToLCD = 0;
		lcd_clrscr();
		Menu_Navigate(&Menu_1);
	}
	else
	{
		DisableButtons();
		StartDebounceTimer();
		
		//lcd_clrscr();
		//lcd_puts("Up\n");
		printf_P(PSTR("Up\n"));
		Menu_Navigate(MENU_PREVIOUS);
	}*/
}

ISR(INT5_vect)
{
	HandleButtonPress(5);	//Center
	/*
	if(OutputTimeToLCD == 1)
	{
		OutputTimeToLCD = 0;
		lcd_clrscr();
		Menu_Navigate(&Menu_1);
	}
	else
	{
		DisableButtons();
		StartDebounceTimer();
		
		//lcd_clrscr();
		//lcd_puts("Center\n");
		printf_P(PSTR("Center\n"));
		Menu_EnterCurrentItem();
	}*/
}

ISR(PCINT1_vect)
{
	if((PINC & 0x04) == 0x00)
	{
		HandleButtonPress(4);	//Down
		/*
		if(OutputTimeToLCD == 1)
		{
			OutputTimeToLCD = 0;
			lcd_clrscr();
			Menu_Navigate(&Menu_1);
		}
		else
		{
			DisableButtons();
			StartDebounceTimer();
			
			//lcd_clrscr();
			//lcd_puts("Down\n");
			printf_P(PSTR("Down\n"));
			Menu_Navigate(MENU_NEXT);
		}*/
	}
	else if((PIND & 0x20) == 0x00)
	{
		HandleButtonPress(2);	//Right
		/*
		if(OutputTimeToLCD == 1)
		{
			OutputTimeToLCD = 0;
			lcd_clrscr();
			Menu_Navigate(&Menu_1);
		}
		else
		{
			DisableButtons();
			StartDebounceTimer();
			
			//lcd_clrscr();
			//lcd_puts("Right\n");
			printf_P(PSTR("Right\n"));
			Menu_Navigate(MENU_CHILD);
		}*/
	}
}

//Timer 1 is used for debouncing
//This timer is started when a button is pressed
//This interrupt will trigger 250ms later, and re-enable the buttons
ISR(TIMER1_COMPA_vect)
{
	EnableButtons();
}

ISR(TIMER1_OVF_vect)
{
	if(ButtonInputTimeoutCount > BUTTON_DELAY_COUNT)
	{
		TCCR1B &= 0xF8;		//Disable timer 1
		//Switch LCD back to idle state
		lcd_clrscr();
		lcd_puts("Idle\n");
		ButtonInputTimeoutCount = 0;
		OutputTimeToLCD = 1;
	}
	else
	{
		ButtonInputTimeoutCount++;
	}
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
					TheTime.hour = 1;
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
	//put time on the lcd screen
	if(OutputTimeToLCD == 1)
	{
		lcd_gotoxy(0, 1);
		if(TheTime.hour > 12)
		{
			fprintf(&LCDStream, "%02u:%02u:%02u PM\n", TheTime.hour-12, TheTime.min, TheTime.sec);
		}
		else
		{
			fprintf(&LCDStream, "%02u:%02u:%02u AM\n", TheTime.hour, TheTime.min, TheTime.sec);
		}
	}
	
	
	}
}

/** @} */
