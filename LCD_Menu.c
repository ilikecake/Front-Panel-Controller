

//#include "lcd.h"
#include "main.h"


uint8_t LCD_Menu_Status;
uint8_t LCD_Menu_Button_Status;




void LCDMenuHardwareInit(void)
{
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

	//Initalize and clear the LCD
	lcd_init(LCD_DISP_ON);
	lcd_clrscr();
	
	EnableButtons();
	
	return;
}





void LCDMenuInit(void)
{
	LCD_Menu_Status = LCD_MENU_STATUS_IDLE;
	LCD_Menu_Button_Status = LCD_MENU_BUTTON_NONE;
	return;
}

void LCDMenuHandle(void)
{
	if(LCD_Menu_Button_Status > 0)
	{

		fprintf(&LCDStream, "%u\n", LCD_Menu_Button_Status);
		LCD_Menu_Button_Status = LCD_MENU_BUTTON_NONE;
	}
	return;
}


void LCDMenuButtonPressed(uint8_t Button)
{
	if(LCD_Menu_Button_Status == LCD_MENU_BUTTON_NONE)
	{
		LCD_Menu_Button_Status = Button;
	}
	return;
}



//Returns the menu status
uint8_t LCDMenuStatus(void)
{
	return LCD_Menu_Status;
}

//Go to idle state
void LCDGotoIdle(void)
{
	TCCR1B &= 0xF8;					//Disable timer 1
	//ButtonInputTimeoutCount = 0;
	//OutputTimeToLCD = 1;
	return;
}

//Start timer to go to idle state
uint8_t LCDStartTimeout(void);

//Stop timer to go to idle state
uint8_t LCDStopTimeout(void);