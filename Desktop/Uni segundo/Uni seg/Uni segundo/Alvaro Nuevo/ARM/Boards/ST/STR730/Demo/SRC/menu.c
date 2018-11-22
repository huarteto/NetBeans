/******************** (C) COPYRIGHT 2005 STMicroelectronics ********************
* File Name          : menu.c
* Author             : MCD Application Team
* Date First Issued  : 09/27/2005 :  V1.0
* Description        : This file provides a set of functions needed to manage the
*                      demonstration menu.
********************************************************************************
* History:
* 09/27/2005 :  V1.0
********************************************************************************
 THE PRESENT SOFTWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS WITH
 CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
 AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT, INDIRECT
 OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE CONTENT
 OF SUCH SOFTWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING INFORMATION
 CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

/* Standard include ----------------------------------------------------------*/
#include "main.h"

/* Include of other module interface headers ---------------------------------*/
/* Local includes ------------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
typedef void (* tMenuFunc)(void);
typedef struct sMenuItem * tMenuItem;
typedef struct sMenu * tMenu;

/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
u8 MenuItemIndex = 0, nMenuLevel = 0;
u8 ItemNumb[MAX_MENU_LEVELS];

tMenuItem psMenuItem, psCurrentMenuItem;
tMenu psPrevMenu[MAX_MENU_LEVELS];
tMenu psCurrentMenu;

struct sMenuItem
{
  u8* pszTitle;
  tMenuFunc pfMenuFunc;
  tMenuFunc pfBackMenuFunc;
  tMenu psSubMenu ;
};

struct sMenu
{
  u8* pszTitle;
  tMenuItem psItems;
  u8 nItems;
};


/* menu level 4------------------------------------------------------------------------*/
struct sMenuItem STOPMenu_PinItems[] = {"WAKE-UP: exit", IdleFunc, IdleFunc};
struct sMenu STOPMenu_Pin = {"STOP Mode", STOPMenu_PinItems, countof(STOPMenu_PinItems)};

struct sMenuItem STOPMenu_WUTItems[] = {"Wakeup in 10s", IdleFunc, IdleFunc};
struct sMenu STOPMenu_WUT = {"STOP Mode", STOPMenu_WUTItems, countof(STOPMenu_WUTItems)};

struct sMenuItem TimeShowMenuItems[] = {"     :  :      ", IdleFunc, IdleFunc};
struct sMenu TimeShowMenu = {"Time Show", TimeShowMenuItems, countof(TimeShowMenuItems)};

struct sMenuItem TimeAdjustMenuItems[] = {"Time Adjust", IdleFunc, IdleFunc};
struct sMenu TimeAdjustMenu = {"Time Adjust", TimeAdjustMenuItems, countof(TimeAdjustMenuItems)};

struct sMenuItem DateShowMenuItems[] = {"     /  /      ", IdleFunc, IdleFunc};
struct sMenu DateShowMenu = {"Date Show", DateShowMenuItems, countof(DateShowMenuItems)};

struct sMenuItem DateAdjustMenuItems[] = {"Date Adjust", IdleFunc, IdleFunc};
struct sMenu DateAdjustMenu = {"Date Adjust", DateAdjustMenuItems, countof(DateAdjustMenuItems)};

struct sMenuItem AlarmShowMenuItems[] = {"     :  :      ", IdleFunc, IdleFunc};
struct sMenu AlarmShowMenu = {"Alarm Show", AlarmShowMenuItems, countof(AlarmShowMenuItems)};

struct sMenuItem AlarmAdjustMenuItems[] = {"Alarm Adjust", IdleFunc, IdleFunc};
struct sMenu AlarmAdjustMenu = {"Alarm Adjust", AlarmAdjustMenuItems, countof(AlarmAdjustMenuItems)};

struct sMenuItem SingleRepeatMenuItems[] = { {"Melody1", PlayMelody1, StopMelody},
                                             {"Melody2", PlayMelody2, StopMelody},
                                             {"Melody3", PlayMelody3, StopMelody}};
struct sMenu SingleRepeatMenu = {"Single Repeat", SingleRepeatMenuItems, countof(SingleRepeatMenuItems)};

struct sMenuItem SingleMenuItems[] = { {"Melody1", PlayMelody1, StopMelody},
                                       {"Melody2", PlayMelody2, StopMelody},
                                       {"Melody3", PlayMelody3, StopMelody}};
struct sMenu SingleMenu = {"Single", SingleMenuItems, countof(SingleMenuItems)};

/*  menu level 3------------------------------------------------------------------------*/

struct sMenuItem SettingDateMenuItems[] = { {"Adjust", Date_Adjust, IdleFunc, &DateAdjustMenu},
                                            {"Show", Date_Show, IdleFunc, &DateShowMenu}};
struct sMenu DateMenu = {"Date", SettingDateMenuItems, countof(SettingDateMenuItems)};

struct sMenuItem SettingTimeMenuItems[] = { {"Adjust", Time_Adjust, IdleFunc, &TimeAdjustMenu},
                                            {"Show", Time_Show, IdleFunc, &TimeShowMenu}};
struct sMenu TimeMenu = {"Time", SettingTimeMenuItems, countof(SettingTimeMenuItems)};

struct sMenuItem SettingAlarmMenuItems[] = { {"Adjust", Alarm_Adjust, IdleFunc, &AlarmAdjustMenu},
                                             {"Show", Alarm_Show, IdleFunc, &AlarmShowMenu}};
struct sMenu AlarmMenu = {"Alarm", SettingAlarmMenuItems, countof(SettingAlarmMenuItems)};

struct sMenuItem STOPMenuItems[] = { {"exit by pin", EnterStopMode, IdleFunc, &STOPMenu_Pin},
                                     {"exit by WUT", EnterStopMode_WUT, IdleFunc, &STOPMenu_WUT}};
struct sMenu STOPMenu = {"STOP Mode", STOPMenuItems, countof(STOPMenuItems)};

struct sMenuItem HALTMenuItems[] = {"RST: exit", IdleFunc, IdleFunc};
struct sMenu HALTMenu = {"HALT Mode", HALTMenuItems, countof(HALTMenuItems)};

struct sMenuItem RUNMenuItems[] = { {"32 MHz", SetClockTo32, SetClockTo32},
                                    {"16 MHz", SetClockTo16, SetClockTo32},
                                    {"8 MHz", SetClockTo8, SetClockTo32},
                                    {"4 MHz", SetClockTo4,SetClockTo32}};
struct sMenu RUNMenu = {"RUN Mode", RUNMenuItems, countof(RUNMenuItems)};

struct sMenuItem CelsiusMenuItems[] = {"            C   ", IdleFunc, Converter_DeInit};
struct sMenu CelsiusShowMenu = {"Temperature     ", CelsiusMenuItems, countof(CelsiusMenuItems)};

struct sMenuItem FahrenheitMenuItems[] = {"            F   ", IdleFunc, Converter_DeInit};
struct sMenu FahrenheitShowMenu = {"Temperature     ", FahrenheitMenuItems, countof(FahrenheitMenuItems)};

struct sMenuItem StartTestItems[] = {"", BoardTest, IdleFunc};
struct sMenu StartTestMenu = {"", StartTestItems, countof(StartTestItems)};

struct sMenuItem TestReportItems[] = {"", TestReport, IdleFunc};
struct sMenu TestReportMenu = {"", TestReportItems, countof(TestReportItems)};

/*  menu level 2------------------------------------------------------------------------*/
struct sMenuItem LowPwMenuItems[] = { {"STOP", IdleFunc, IdleFunc, &STOPMenu},
                                      {"HALT", EnterHALTMode, IdleFunc, &HALTMenu},
                                      {"RUN", IdleFunc, IdleFunc, &RUNMenu}};
struct sMenu LowPwMenu = {"Low pw modes", LowPwMenuItems, countof(LowPwMenuItems)};

struct sMenuItem SettingsMenuItems[] = { {"Time", IdleFunc, IdleFunc, &TimeMenu},
                                         {"Date", IdleFunc, IdleFunc, &DateMenu},
                                         {"Alarm", IdleFunc, IdleFunc, &AlarmMenu}};
struct sMenu SettingsMenu = {"Time Setting", SettingsMenuItems, countof(SettingsMenuItems)};

struct sMenuItem UnitDisplayMenuItems[] = { {"Celsius Unit    ", CelsiusDisplay, IdleFunc, &CelsiusShowMenu},
                                            {"Fahrenheit Unit ", FahrenheitDisplay, IdleFunc, &FahrenheitShowMenu}};
struct sMenu UnitDisplayMenu = {"Temperature     ", UnitDisplayMenuItems, countof(UnitDisplayMenuItems)};

struct sMenuItem PlayModeMenuItems[] = { {"Single", Single, IdleFunc, &SingleMenu},
                                         {"Single Repeat", SingleRepeat, IdleFunc, &SingleRepeatMenu}};
struct sMenu PlayModeMenu = {"Melodies Play", PlayModeMenuItems, countof(PlayModeMenuItems)};

struct sMenuItem PeriodMeasureItems[] = {"Period Measure  ", IdleFunc, IdleFunc};
struct sMenu PeriodMeasureMenu = {"Period Measure", PeriodMeasureItems, countof(PeriodMeasureItems)};

struct sMenuItem BoardTestItems[] = { {"Start Test      ", BoardTest, IdleFunc, &StartTestMenu},
                                      {"Test report     ", TestReport, IdleFunc, &TestReportMenu}};
struct sMenu BoardTestMenu = {"Board Test      ", BoardTestItems, countof(BoardTestItems)};

/*  menu level 1 ------------------------------------------------------------------------*/
struct sMenuItem MainMenuItems[] = {
  {"Time Setting    ", IdleFunc, IdleFunc, &SettingsMenu},
  {"Low pw modes    ", IdleFunc, IdleFunc, &LowPwMenu},
  {"Temperature     ", Converter_Init, IdleFunc, &UnitDisplayMenu},
  {"Melodies        ", Melody_Init, IdleFunc, &PlayModeMenu},
  {"Period Measure  ", PeriodPulseMeasure, IdleFunc, &PeriodMeasureMenu},
  {"Board Test      ", IdleFunc, IdleFunc, &BoardTestMenu},
};
struct sMenu MainMenu = {"Main menu", MainMenuItems, countof(MainMenuItems)};

/* Private function prototypes -----------------------------------------------*/
/* Interface functions -------------------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/*******************************************************************************
* Function Name  : Menu_Init
* Description    : Initializes the demonstration menu.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void Menu_Init(void)
{
  psCurrentMenu = &MainMenu;
  psPrevMenu[nMenuLevel] = psCurrentMenu;
  psMenuItem = MainMenuItems;
}

/*******************************************************************************
* Function Name  : DisplayMenu
* Description    : Displays the current menu.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void DisplayMenu(void)
{
  LCD_SendString("                ", 1, 1);
  LCD_SendString("                ", 2, 1);
  LCD_SendString(psCurrentMenu->pszTitle, 1, 1);
  LCD_SendString(psMenuItem->pszTitle, 2, 2);
}

/*******************************************************************************
* Function Name  : SelFunc
* Description    : This function is executed when SEL push-buttton is pressed.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void SelFunc(void)
{
  psCurrentMenuItem = psMenuItem;

  if(psMenuItem->psSubMenu != NULL)
  {
   /* update the current Item by the submenu */
   MenuItemIndex = 0;
   psCurrentMenu = psMenuItem->psSubMenu;
   psMenuItem = &(psCurrentMenu->psItems)[MenuItemIndex];
   DisplayMenu();
   nMenuLevel++;
   psPrevMenu[nMenuLevel] = psCurrentMenu;
  }

  psCurrentMenuItem->pfMenuFunc();
}

/*******************************************************************************
* Function Name  : NxtFunc
* Description    : This function is executed when NEXT push-buttton is pressed.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void NxtFunc(void)
{
  /* test on the MenuItemIndex value before incrementing it */
  if(MenuItemIndex >= ((psCurrentMenu->nItems)-1))
  {
    MenuItemIndex = 0;
  } 
  else
  {
    MenuItemIndex++;
  }
  
  /* get the current menu */
  psMenuItem = &psCurrentMenu->psItems[MenuItemIndex];

  DisplayMenu();
  
  ItemNumb[nMenuLevel] = MenuItemIndex;
}

/*******************************************************************************
* Function Name  : PrvFunc
* Description    : This function is executed when PREV push-buttton is pressed.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void PrvFunc(void)
{
  if(MenuItemIndex > 0)
  {
    MenuItemIndex--;
  }
  else
  {
    MenuItemIndex = psCurrentMenu->nItems - 1;
  }
  
  psMenuItem = &psCurrentMenu->psItems[MenuItemIndex];
  DisplayMenu();
  ItemNumb[nMenuLevel] = MenuItemIndex;
}

/*******************************************************************************
* Function Name  : BackFunc
* Description    : This function is executed when BACK push-buttton is pressed.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void BackFunc(void)
{
  psMenuItem->pfBackMenuFunc();

  if(nMenuLevel == 0)
  {
    nMenuLevel++;
  }
 
  psCurrentMenu = psPrevMenu[nMenuLevel-1];
  psMenuItem = &psCurrentMenu->psItems[ItemNumb[nMenuLevel-1]];
  DisplayMenu();
  ItemNumb[nMenuLevel] = 0;
  MenuItemIndex = ItemNumb[nMenuLevel-1];
  nMenuLevel--;
}

/*******************************************************************************
* Function Name  : ReadKey
* Description    : Reads key from demoboard.
* Input          : None
* Output         : None
* Return         : Return NXT, PRV, SEL or NOKEY
*******************************************************************************/
u8 ReadKey ( void )
{ 
  if(GPIO_BitRead(GPIO5,GPIO_PIN_8))
  {
    while(GPIO_BitRead(GPIO5,GPIO_PIN_8) == 1);
    return NXT; /* NEXT key is pressed */
  }

  if(GPIO_BitRead(GPIO5,GPIO_PIN_9))
  {
    while(GPIO_BitRead(GPIO5,GPIO_PIN_9) == 1);
    return PRV; /* PREV key is pressed */
  }
  
  if(GPIO_BitRead(GPIO5,GPIO_PIN_10))
  {
    while(GPIO_BitRead(GPIO5,GPIO_PIN_10) == 1);
    return SEL; /* SEL key is pressed */
  }
  
  if(GPIO_BitRead(GPIO5,GPIO_PIN_11))
  {
    while(GPIO_BitRead(GPIO5,GPIO_PIN_11) == 1);
    return BACK; /* BACK key is pressed */
  }
  else /* no key is pressed */
  {
    return NOKEY;
  }
}

/*******************************************************************************
* Function Name  : IdleFunc
* Description    : Idle function.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void IdleFunc(void) 
{
  return;
}

/******************* (C) COPYRIGHT 2005 STMicroelectronics *****END OF FILE****/

