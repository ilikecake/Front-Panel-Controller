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
*	\brief		Project specific config options for the standalone timer module
*	\author		Pat Satyshur
*	\version	1.1
*	\date		1/20/2013
*	\copyright	Copyright 2013, Pat Satyshur
*	\ingroup 	beer_heater_v1_main
*
*	@{
*/

//Global config options
//Put options here to configure the common modules.

#ifndef _CONFIG_H_
#define _CONFIG_H_

#include <avr/pgmspace.h>

//Setup for the command interpreter
#define COMMAND_USER_CONFIG
#define COMMAND_STAT_SHOW_COMPILE_STRING			1		//Set to 1 to output the compile date/time string in the stat function					
#define COMMAND_STAT_SHOW_MEM_USAGE					1		//Set to 1 to show the memory usage in the stat function. NOTE: if this is enabled, the mem_usage.c must be included in the makefile

//Based on the setup above
#if COMMAND_STAT_SHOW_COMPILE_STRING == 1
	extern const char fwCompileDate[] PROGMEM;				//The compile date/time string. This must be a string in flash called fwCompileDate.
#endif

#if COMMAND_STAT_SHOW_MEM_USAGE == 1
	#include "mem_usage.h"									//The header that contains StackCount()
#endif


//Setup for the I2C software driver
#define I2C_SOFT_USER_CONFIG
#define I2C_SOFT_USE_INTERNAL_PULLUPS		1		//Set to 1 to use internal pullups on the pins
#define I2C_SOFT_USE_ARBITRATION			1		//Set to 1 to enable arbitration
#define I2C_SOFT_USE_CLOCK_STRETCH			1		//Set to 1 to enable clock stretching detection
#define I2C_SOFT_CLOCK_STRETCH_TIMEOUT		1000	//The timeout for the clock stretching, this is a 16-bit number

#define I2C_SDA_PORT			PORTC
#define I2C_SDA_DDR				DDRC
#define I2C_SDA_PIN				PINC
#define I2C_SDA_PIN_NUM			6
#define I2C_SCL_PORT			PORTB
#define I2C_SCL_DDR				DDRB
#define I2C_SCL_PIN				PINB
#define I2C_SCL_PIN_NUM			7

#endif

/** @} */
