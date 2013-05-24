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
*	\brief		Command interpreter application specific functions
*	\author		Pat Satyshur
*	\version	1.1
*	\date		1/13/2013
*	\copyright	Copyright 2013, Pat Satyshur
*	\ingroup 	beer_heater_main
*
*	@{
*/

#include "main.h"
//#include "commands.h"


//The number of commands
const uint8_t NumCommands = 11;

//Handler function declerations

//LED control function
static int _F1_Handler (void);
const char _F1_NAME[] PROGMEM 			= "lcdclr";
const char _F1_DESCRIPTION[] PROGMEM 	= "clear the LCD";
const char _F1_HELPTEXT[] PROGMEM 		= "'lcdclr' has no parameters";

//Jump to DFU bootloader
static int _F2_Handler (void);
const char _F2_NAME[] PROGMEM 			= "dfu";
const char _F2_DESCRIPTION[] PROGMEM 	= "Jump to bootloader";
const char _F2_HELPTEXT[] PROGMEM 		= "'dfu' has no parameters";

//Read a register
static int _F3_Handler (void);
const char _F3_NAME[] PROGMEM 			= "button";
const char _F3_DESCRIPTION[] PROGMEM 	= "Enable/disable the buttons";
const char _F3_HELPTEXT[] PROGMEM 		= "button <state>";

//Set time on the internal timer
static int _F4_Handler (void);
const char _F4_NAME[] PROGMEM 			= "settime";
const char _F4_DESCRIPTION[] PROGMEM 	= "Set the time";
const char _F4_HELPTEXT[] PROGMEM 		= "settime <year> <month> <day> <dow> <hr> <min> <sec>";

//Read the time from the internal timer
static int _F5_Handler (void);
const char _F5_NAME[] PROGMEM 			= "gettime";
const char _F5_DESCRIPTION[] PROGMEM 	= "Get the time from the internal timer";
const char _F5_HELPTEXT[] PROGMEM 		= "'gettime' has not parameters";

//Write a register
static int _F6_Handler (void);
const char _F6_NAME[] PROGMEM 			= "lcdwrite";
const char _F6_DESCRIPTION[] PROGMEM 	= "write to the lcd";
const char _F6_HELPTEXT[] PROGMEM 		= "lcdwrite <data>";

//Get a set of data from the devices
static int _F8_Handler (void);
const char _F8_NAME[] PROGMEM 			= "bkl";
const char _F8_DESCRIPTION[] PROGMEM 	= "Turn the backlight on/off";
const char _F8_HELPTEXT[] PROGMEM 		= "bkl <state>";

//Read a register from the memory
static int _F9_Handler (void);
const char _F9_NAME[] PROGMEM 			= "test";
const char _F9_DESCRIPTION[] PROGMEM 	= "test function";
const char _F9_HELPTEXT[] PROGMEM 		= "nothing yet";

//Pressure sensor functions
static int _F10_Handler (void);
const char _F10_NAME[] PROGMEM 			= "pres";
const char _F10_DESCRIPTION[] PROGMEM 	= "Pressure sensor functions";
const char _F10_HELPTEXT[] PROGMEM 		= "pres <function>";

//Humidity sensor functions
static int _F11_Handler (void);
const char _F11_NAME[] PROGMEM 			= "rh";
const char _F11_DESCRIPTION[] PROGMEM 	= "Humidity sensor functions";
const char _F11_HELPTEXT[] PROGMEM 		= "rh <cnd> <val>";

//Scan the TWI bus for devices
static int _F12_Handler (void);
const char _F12_NAME[] PROGMEM 			= "twiscan";
const char _F12_DESCRIPTION[] PROGMEM 	= "Scan for TWI devices";
const char _F12_HELPTEXT[] PROGMEM 		= "'twiscan' has no parameters";

//Command list
const CommandListItem AppCommandList[] PROGMEM =
{
	{ _F1_NAME,		0,  0,	_F1_Handler,	_F1_DESCRIPTION,	_F1_HELPTEXT	},		//lcdclr
	{ _F2_NAME, 	0,  0,	_F2_Handler,	_F2_DESCRIPTION,	_F2_HELPTEXT	},		//dfu
	{ _F3_NAME, 	1,  1,	_F3_Handler,	_F3_DESCRIPTION,	_F3_HELPTEXT	},		//button
	{ _F4_NAME, 	7,  7,	_F4_Handler,	_F4_DESCRIPTION,	_F4_HELPTEXT	},		//settime
	{ _F5_NAME, 	0,  0,	_F5_Handler,	_F5_DESCRIPTION,	_F5_HELPTEXT	},		//gettime
	{ _F6_NAME, 	1,  1,	_F6_Handler,	_F6_DESCRIPTION,	_F6_HELPTEXT	},		//lcdwrite	
	{ _F8_NAME,		1,  1,	_F8_Handler,	_F8_DESCRIPTION,	_F8_HELPTEXT	},		//bkl
	{ _F9_NAME,		0,  3,	_F9_Handler,	_F9_DESCRIPTION,	_F9_HELPTEXT	},		//test
	{ _F10_NAME,	0,  0,	_F10_Handler,	_F10_DESCRIPTION,	_F10_HELPTEXT	},		//pres
	{ _F11_NAME,	1,  2,	_F11_Handler,	_F11_DESCRIPTION,	_F11_HELPTEXT	},		//rh
	{ _F12_NAME,	0,  0,	_F12_Handler,	_F12_DESCRIPTION,	_F12_HELPTEXT	},		//twiscan
};

//Command functions

//Clear LCD screen
static int _F1_Handler (void)
{
	lcd_clrscr();
	return 0;
}

//Jump to DFU bootloader
static int _F2_Handler (void)
{
	printf_P(PSTR("Jumping to bootloader. A manual reset will be required\nPress 'y' to continue..."));
	
	if(WaitForAnyKey() == 'y')
	{
		printf_P(PSTR("Jump\n"));
		DelayMS(100);
		Jump_To_Bootloader();
	}
	
	printf_P(PSTR("Canceled\n"));
	return 0;
}

//Enable or disable the buttons
static int _F3_Handler (void)
{
	uint8_t NewButtonState;
	
	NewButtonState = argAsInt(1);
	if(NewButtonState == 1)
	{
		EnableButtons();
	}
	else if(NewButtonState == 0)
	{
		DisableButtons();
	}
	return 0;
}

//Set time on the internal timer
static int _F4_Handler (void)
{
	TimeAndDate CurrentTime;
	//<year> <month> <day> <dow> <hr> <min> <sec>
	CurrentTime.year	= argAsInt(1);
	CurrentTime.month	= argAsInt(2);
	CurrentTime.day		= argAsInt(3);
	CurrentTime.dow		= argAsInt(4);
	CurrentTime.hour	= argAsInt(5);
	CurrentTime.min		= argAsInt(6);
	CurrentTime.sec		= argAsInt(7);
	SetTime(CurrentTime);
	printf_P(PSTR("Setting %02u/%02u/%04u %02u:%02u:%02u"), CurrentTime.month, CurrentTime.day, CurrentTime.year, CurrentTime.hour, CurrentTime.min, CurrentTime.sec);
	
	printf_P(PSTR("......Done\n"));
	return 0;
}

//Read the time from the internal timer
static int _F5_Handler (void)
{
	TimeAndDate CurrentTime;
	GetTime(&CurrentTime);
	printf_P(PSTR("%02u/%02u/%04u %02u:%02u:%02u\n"), CurrentTime.month, CurrentTime.day, CurrentTime.year, CurrentTime.hour, CurrentTime.min, CurrentTime.sec);
	return 0;
}

//Write a register
static int _F6_Handler (void)
{
	char DataToWrite[16];
	argAsChar(1, DataToWrite);
	
	lcd_puts(DataToWrite);
	lcd_puts("\n");
	/*if(tcs3414_WriteReg(RegToWrite, DataToWrite) == 0)
	{
		printf_P(PSTR("OK\n"));
	}
	else
	{
		printf_P(PSTR("Error\n"));
	}*/

	return 0;
}

//Get a set of data from the devices
static int _F8_Handler (void)
{
	uint8_t NewState = argAsInt(1);
	
	if(NewState == 1)
	{
		PORTB |= (1<<6);
	}
	else
	{
		PORTB &= ~(1<<6);
	}
	return 0;
}

//Read a register from the memory
static int _F9_Handler (void)
{
	uint8_t CmdState = argAsInt(1);
	TimeAndDate CurrentTime;
	

	switch(CmdState)
	{
		case 1:
			lcd_clrscr();
			break;
			
		case 2:
			fprintf(&LCDStream, "test1");
			break;
			
		case 3:
			lcd_gotoxy(0, 1);
			fprintf(&LCDStream, "test2");
			break;
	
		case 4:
			GetTime(&CurrentTime);
			lcd_init(LCD_DISP_ON_CURSOR);
			fprintf(&LCDStream, "Set Time\n %02u:%02u:%02u\n", CurrentTime.hour, CurrentTime.min, CurrentTime.sec);
			lcd_gotoxy(2, 1);
			break;
	
	}

	return 0;
}

//Pressure sensor functions
static int _F10_Handler (void)
{
	//int16_t Pressure_kPa;
	/*MPL115A1_GetPressure(&Pressure_kPa);
	printf_P(PSTR("Pressure: %u.%u kPa\n"), ((int16_t)Pressure_kPa)>>4, ((((int16_t)Pressure_kPa)&0x000F)*1000)/(16) );*/
	return 0;
}

//Humidity sensor functions
static int _F11_Handler (void)
{
	/*uint8_t temp;
	uint8_t stat;
	uint8_t InputCmd	= argAsInt(1);
	uint8_t InputVal	= argAsInt(2);
	int16_t RecievedData;
	
	uint16_t SNA;
	uint32_t SNB;
	uint16_t SNC;*/
	
	/*if(InputCmd == 1)
	{
		stat = SHT25_ReadUserReg(&temp);
		if(stat != SOFT_I2C_STAT_OK)
		{
			printf_P(PSTR("ERROR: 0x%02X\n"), stat);
		}
		else
		{
			printf_P(PSTR("REG: 0x%02X\n"), temp);
		}
		return 0;
	}
	else if(InputCmd == 2)
	{
		printf_P(PSTR("Writing 0x%02X..."), InputVal);
		stat = SHT25_WriteUserReg(InputVal);
		printf_P(PSTR("0x%02X\n"), stat);
		return 0;
	}
	else if(InputCmd == 3)
	{
		printf_P(PSTR("Device reset\n"));
		SHT25_Reset();
	}
	else if(InputCmd == 4)
	{
		stat = SHT25_ReadTemp(&RecievedData);
		if(stat == SHT25_RETURN_STATUS_OK)
		{
			printf_P(PSTR("Temp %d.%02u C\n"), RecievedData/100, RecievedData%100);
		}
		else if(stat == SHT25_RETURN_STATUS_CRC_ERROR)
		{
			printf_P(PSTR("CRC Error\n"));
		}
		else
		{
			printf_P(PSTR("Timeout\n"));
		}
	}
	else if(InputCmd == 5)
	{
		stat = SHT25_ReadRH(&RecievedData);
		if(stat == SHT25_RETURN_STATUS_OK)
		{
			printf_P(PSTR("RH: %u.%02u%%\n"), RecievedData/100, RecievedData%100);
		}
		else if(stat == SHT25_RETURN_STATUS_CRC_ERROR)
		{
			printf_P(PSTR("CRC Error\n"));
		}
		else
		{
			printf_P(PSTR("Timeout\n"));
		}
	}
	else if(InputCmd == 6)
	{
		stat = SHT25_ReadID(&SNA, &SNB, &SNC);
		if(stat == SHT25_RETURN_STATUS_OK)
		{
			printf_P(PSTR("SN: 0x%04X%08lX%04X\n"), SNA, SNB, SNC);
		}
		else if(stat == SHT25_RETURN_STATUS_CRC_ERROR)
		{
			printf_P(PSTR("CRC Error\n"));
		}
		else
		{
			printf_P(PSTR("I2C error\n"));
		}
	}*/
	
	return 0;
}

//Scan the TWI bus for devices
static int _F12_Handler (void)
{
	//I2CSoft_Scan();
	return  0;
}

/** @} */
