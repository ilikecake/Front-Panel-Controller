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
*	\brief		Hardware definitions and functions header file for the environmental sensor.
*	\author		Pat Satyshur
*	\version	1.0
*	\date		2/3/2013
*	\copyright	Copyright 2013, Pat Satyshur
*	\ingroup 	hardware
*
*	\defgroup	hardware Hardware Drivers
*
*	@{
*/

#ifndef _HARDWARE_H_
#define _HARDWARE_H_

/** initalizes the hardware used for the environmental sensor
*	- GPIO directions.
*	- Timer 0 interrupts every 1ms for timing functions.
*/
void HardwareInit( void );

void DelayMS(uint16_t ms);
//void DelaySEC(uint16_t SEC);
void GetTime( TimeAndDate *time );
void SetTime( TimeAndDate time );

void EnableButtons(void);
void DisableButtons(void);

void StartTimer(void);
void StopTimer(void);
/*void RestartTimer(uint16_t *FinalMS, uint16_t *FinalSEC);*/

void LED(uint8_t LEDState);


//LCD Functions

//Returns the menu status
uint8_t LCDMenuStatus(void);

//Go to idle state
//uint8_t LCDGotoIdle(void);

//Start timer to go to idle state
uint8_t LCDStartTimeout(void);

//Stop timer to go to idle state
uint8_t LCDStopTimeout(void);

void HandleButtonPress(void);




//Returns 1 if the year is a leap year
uint8_t IsLeapYear(uint16_t TheYear);

//Returns the number of days in the month. Will always return 28 for February, aditional checks will be needed to correct for leap years.
uint8_t DaysPerMonth(uint8_t MonthNumber);

#endif

/** @} */
