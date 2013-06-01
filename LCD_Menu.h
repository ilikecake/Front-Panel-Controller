


#define LCD_MENU_STATUS_IDLE	0

#define LCD_MENU_BUTTON_NONE		0
#define LCD_MENU_BUTTON_UP			1
#define LCD_MENU_BUTTON_DOWN		2
#define LCD_MENU_BUTTON_LEFT		3
#define LCD_MENU_BUTTON_RIGHT		4
#define LCD_MENU_BUTTON_CENTER		5


//Hardware specific functions
void LCDMenuHardwareInit(void);


//LCD Functions

void LCDMenuInit(void);

void LCDMenuHandle(void);

void LCDMenuButtonPressed(uint8_t Button);

//Returns the menu status
uint8_t LCDMenuStatus(void);

//Go to idle state
void LCDGotoIdle(void);

//Start timer to go to idle state
uint8_t LCDStartTimeout(void);

//Stop timer to go to idle state
uint8_t LCDStopTimeout(void);