/******************** (C) COPYRIGHT 2006 STMicroelectronics ********************
* File Name          : menu.c
* Author             : MCD Application Team
* Date First Issued  : 05/18/2006 : Version 1.0
* Description        : This file includes the menu navigation driver of STR75x-EVAL.
********************************************************************************
* History:
* 07/20/2006 : Version 1.2
* 05/24/2006 : Version 1.1
* 05/18/2006 : Version 1.0
********************************************************************************
* THE PRESENT SOFTWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH SOFTWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private typedef -----------------------------------------------------------*/
  typedef void (* tMenuFunc)(void);
  typedef struct sMenuItem * tMenuItem;
  typedef struct sMenu * tMenu;

/* Private define ------------------------------------------------------------*/

  /* "SEL" key help master LCD graphic */
  u8 SELHelp[] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x40, 0x60, 0x70, 0xF8, 0xF8, 0x70, 0x60, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xC0, 0xE0, 0xF0,
  0x80, 0x80, 0x80, 0x00, 0x00, 0x00, 0xC0, 0x20, 0x20, 0x20, 0x00, 0x00, 0xE3,
  0x23, 0x20, 0x20, 0x00, 0x00, 0xE0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x80,
  0x80, 0xF0, 0xE0, 0xC0, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x03, 0x07, 0x0F, 0x01, 0x01, 0x01, 0x00,
  0x00, 0x00, 0x08, 0x09, 0x09, 0x06, 0x00, 0x00, 0xCF, 0xC9, 0x09, 0x08, 0x00,
  0x00, 0x0F, 0x08, 0x08, 0x08, 0x00, 0x00, 0x01, 0x01, 0x01, 0x0F, 0x07, 0x03,
  0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x02, 0x06, 0x0E, 0x1F, 0x1F, 0x0E, 0x06, 0x02, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

  /* "UP" and "DOWN" keys help master LCD graphic */
  u8 UPDOWNHelp[] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x40, 0x60, 0x70, 0xF8, 0xF8, 0x70, 0x60, 0x40, 0x00, 0x00, 0x78, 0x80, 0x80,
  0x78, 0x00, 0xF8, 0x28, 0x28, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0x20, 0x20, 0x20, 0x00, 0x00, 0xE3,
  0x23, 0x20, 0x20, 0x00, 0x00, 0xE0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x08, 0x09, 0x09, 0x06, 0x00, 0x00, 0xCF, 0xC9, 0x09, 0x08, 0x00,
  0x00, 0x0F, 0x08, 0x08, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x1F, 0x11, 0x11, 0x0E, 0x00, 0x0E, 0x11, 0x11,
  0x0E, 0x00, 0x0F, 0x10, 0x0C, 0x10, 0x0F, 0x00, 0x1F, 0x06, 0x0C, 0x1F, 0x00,
  0x00, 0x02, 0x06, 0x0E, 0x1F, 0x1F, 0x0E, 0x06, 0x02, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

  /* "LEFT" and "RIGHT" keys help master LCD graphic */
  u8 LEFTRIGHTHelp[] = {
  0x00, 0x00, 0x00, 0xF8, 0x80, 0x80, 0x00, 0xF8, 0xA8, 0x88, 0x00, 0xF8, 0x28,
  0x08, 0x00, 0x08, 0xF8, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xC0, 0xE0, 0xF0,
  0x80, 0x80, 0x80, 0x00, 0x00, 0x00, 0xC0, 0x20, 0x20, 0x20, 0x00, 0x00, 0xE0,
  0x20, 0x20, 0x20, 0x00, 0x00, 0xE0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x80,
  0x80, 0xF0, 0xE0, 0xC0, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x03, 0x07, 0x0F, 0x01, 0x01, 0x01, 0x00,
  0x00, 0x00, 0x08, 0x09, 0x09, 0x06, 0x00, 0x00, 0x0F, 0x09, 0x09, 0x08, 0x00,
  0x00, 0x0F, 0x08, 0x08, 0x88, 0x80, 0x80, 0x01, 0x01, 0x81, 0x0F, 0x07, 0x83,
  0x81, 0x00, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x80, 0x80, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x0F, 0x02, 0x06, 0x09, 0x00, 0x0F, 0x00, 0x07, 0x08, 0x0A, 0x06, 0x00, 0x0F,
  0x02, 0x0F, 0x00, 0x00, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00};

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

  extern vu16 PlayIntroDelay;
  u8 MenuItemIndex = 0, nMenuLevel = 0;
  u8 ItemNumb[MAX_MENU_LEVELS];
  u8* CurrentMenu;

  tMenuItem psMenuItem, psCurrentMenuItem;
  tMenu psPrevMenu[MAX_MENU_LEVELS];
  tMenu psCurrentMenu;

  struct sMenuItem
  {
    u8* pszTitle;
    tMenuFunc pfMenuFunc;
    tMenuFunc pfUpDownMenuFunc;
    tMenu psSubMenu ;
  };

  struct sMenu
  {
    u8* pszTitle;
    tMenuItem psItems;
    u8 nItems;
  };


/*------------------------------ Menu level 4 -------------------------------*/
struct sMenuItem TimeShowMenuItems[] = {"     :  :      ", IdleFunc, IdleFunc};
struct sMenu TimeShowMenu = {"    Time Show    ", TimeShowMenuItems, countof(TimeShowMenuItems)};

struct sMenuItem TimeAdjustMenuItems[] = {"    Time Adjust  ", IdleFunc, IdleFunc};
struct sMenu TimeAdjustMenu = {"    Time Adjust  ", TimeAdjustMenuItems, countof(TimeAdjustMenuItems)};

struct sMenuItem DateShowMenuItems[] = {"               ", IdleFunc, IdleFunc};
struct sMenu DateShowMenu = {"    Date Show    ", DateShowMenuItems, countof(DateShowMenuItems)};

struct sMenuItem DateAdjustMenuItems[] = {"    Date Adjust  ", IdleFunc, IdleFunc};
struct sMenu DateAdjustMenu = {"    Date Adjust  ", DateAdjustMenuItems, countof(DateAdjustMenuItems)};

struct sMenuItem AlarmShowMenuItems[] = {"     :  :        ", IdleFunc, IdleFunc};
struct sMenu AlarmShowMenu = {"    Alarm Show   ", AlarmShowMenuItems, countof(AlarmShowMenuItems)};

struct sMenuItem AlarmAdjustMenuItems[] = {"   Alarm Adjust  ", IdleFunc, IdleFunc};
struct sMenu AlarmAdjustMenu = {"   Alarm Adjust  ", AlarmAdjustMenuItems, countof(AlarmAdjustMenuItems)};

/*------------------------------ Menu level 3 -------------------------------*/
struct sMenuItem SettingDateMenuItems[] = { {"     Adjust      ", Date_Adjust, IdleFunc, &DateAdjustMenu},
                                            {"      Show       ", Date_Show, IdleFunc, &DateShowMenu}};
struct sMenu DateMenu = {"      Date       ", SettingDateMenuItems, countof(SettingDateMenuItems)};

struct sMenuItem SettingTimeMenuItems[] = { {"     Adjust      ", Time_Adjust, IdleFunc, &TimeAdjustMenu},
                                            {"      Show       ", Time_Show, IdleFunc, &TimeShowMenu}};
struct sMenu TimeMenu = {"      Time       ", SettingTimeMenuItems, countof(SettingTimeMenuItems)};

struct sMenuItem SettingAlarmMenuItems[] = { {"     Adjust      ", Alarm_Adjust, IdleFunc, &AlarmAdjustMenu},
                                             {"      Show       ", Alarm_Show, IdleFunc, &AlarmShowMenu}};
struct sMenu AlarmMenu = {"      Alarm      ", SettingAlarmMenuItems, countof(SettingAlarmMenuItems)};

struct sMenuItem PromoWaveMenuItems[] = { {"  Play/Replay    ", WavePlayer_Start, IdleFunc},
                                          {"      Stop       ", WavePlayer_Stop, IdleFunc}};
struct sMenu PromoWaveMenu = {"     OSIRIS    ", PromoWaveMenuItems, countof(PromoWaveMenuItems)};


/*------------------------------ Menu level 2 -------------------------------*/
struct sMenuItem CalendarMenuItems[] = { {"      Time       ", IdleFunc, IdleFunc, &TimeMenu},
                                         {"      Date       ", IdleFunc, IdleFunc, &DateMenu},
                                         {"      Alarm      ", IdleFunc, IdleFunc, &AlarmMenu}};
struct sMenu CalendarMenu = {"     Calendar    ", CalendarMenuItems, countof(CalendarMenuItems)};

struct sMenuItem WavePlayerMenuItems[] = {{"     OSIRIS    ", IdleFunc, IdleFunc, &PromoWaveMenu}};
struct sMenu WavePlayerMenu = {"   Wave Player   ",WavePlayerMenuItems, countof(WavePlayerMenuItems)};


struct sMenuItem CANLoopBackItems[] = {{"      Start      ", CANLoopBack_Start, IdleFunc}};
struct sMenu CANLoopBackMenu = {"  CAN Loop Back  ", CANLoopBackItems, countof(CANLoopBackItems)};

struct sMenuItem USBMouseDemoItems[] = {{"      Start      ", JoyStickHID_Start, IdleFunc}};
struct sMenu USBMouseDemoMenu = {" USB Mouse Demo  ", USBMouseDemoItems, countof(USBMouseDemoItems)};

struct sMenuItem WebserverDemoMenuItems[] = {{"      Start      ", Webserver_Start, IdleFunc}};
struct sMenu WebserverDemoMenu = {" Webserver Demo  ", WebserverDemoMenuItems, countof(WebserverDemoMenuItems)};

/*------------------------------ Menu level 1 -------------------------------*/
struct sMenuItem MainMenuItems[] = {
  {"       Help      ", HelpMenu, IdleFunc},
  {"     Calendar    ", IdleFunc, IdleFunc, &CalendarMenu},
  {"   Wave Player   ", IdleFunc, IdleFunc, &WavePlayerMenu},
  {"  CAN Loop Back  ", IdleFunc, IdleFunc, &CANLoopBackMenu},
  {" USB Mouse Demo  ", IdleFunc, IdleFunc, &USBMouseDemoMenu},
  {" Webserver Demo  ", IdleFunc, IdleFunc, &WebserverDemoMenu}};
struct sMenu MainMenu = {"    Main menu    ", MainMenuItems, countof(MainMenuItems)};

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/*******************************************************************************
* Function Name  : Menu_Init
* Description    : Initializes the navigation menu.
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
  LCD_DisplayString(Line1, psCurrentMenu->pszTitle, WhiteText);
  LCD_DisplayString(Line2, psMenuItem->pszTitle, BlackText);
  CurrentMenu = psCurrentMenu->pszTitle;
}

/*******************************************************************************
* Function Name  : SelFunc
* Description    : This function is executed when "SEL" push-buttton is pressed.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void SelFunc(void)
{
  psCurrentMenuItem = psMenuItem;

  if(psMenuItem->psSubMenu != NULL)
  {
   /* Update the current Item by the submenu */
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
* Function Name  : RightFunc
* Description    : This function is executed when "Right" push-buttton is pressed.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void RightFunc(void)
{
  /* Test on the MenuItemIndex value before incrementing it */
  if(MenuItemIndex >= ((psCurrentMenu->nItems)-1))
  {
    MenuItemIndex = 0;
  }
  else
  {
    MenuItemIndex++;
  }

  /* Get the current menu */
  psMenuItem = &(psCurrentMenu->psItems[MenuItemIndex]);

  DisplayMenu();

  ItemNumb[nMenuLevel] = MenuItemIndex;
}

/*******************************************************************************
* Function Name  : LeftFunc
* Description    : This function is executed when "Left" push-buttton is pressed.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void LeftFunc(void)
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
* Function Name  : UpDownFunc
* Description    : This function is executed when any of "UP"/"Down" push-butttons
*                  is pressed.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void UpDownFunc(void)
{
  psMenuItem->pfUpDownMenuFunc();

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
* Return         : Return RIGHT, LEFT, SEL, UP, DOWN or NOKEY
*******************************************************************************/
u8 ReadKey ( void )
{
  u8 port=0;
  u32 i;

  for(i=0x7FFFF; i>0; i--);
  port = GPIO_Read(GPIO7);
  if((port & 0xF0)!= 0x10)
  {
    if((port&0xE0) == 0x80)
      return(UP);
    if((port&0xE0) == 0x20)
      return(DOWN);
    if((port&0xE0) == 0x60)
      return(RIGHT);
    if((port&0xE0) == 0x40)
      return(LEFT);
    if((port&0xE0) == 0x00)
      return(SEL);
    if((port&0xE0) == 0xA0)
      return(KEY);
    else return(NOKEY);
  }
  else return(NOKEY);

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
  /* Nothing to execute: return */
  return;
}

/*******************************************************************************
* Function Name  : HelpMenu
* Description    : Display the Help menu.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void HelpMenu(void)
{
  u8 MyKey = 0;
  PlayIntroDelay = 20;

  VIC_ITCmd(EXTIT3_ITLine, DISABLE);

  /* Wait any bushbuttons press */
  while(MyKey == NOKEY)
  {
    /* Clear the LCD screen */
    LCD_Clear();
    /* Display "         Enter to" message */
    LCD_DisplayString(Line1, "         Enter to", BlackText);
    /* Display "         Sub-Menu" message */
    LCD_DisplayString(Line2, "         Sub-Menu", BlackText);
    /* Display SELHelp graphic */
    LCD_DrawMasterGraphic(SELHelp);
    /* Wait 2sec or return if any pushbutton is pressed */
    while(PlayIntroDelay != 0)
    {
      /* If any key is pressed */
      if(JoyState() != NOKEY)
      {
        /* Display "Help" menu */
        DisplayMenu();
        VIC_ITCmd(EXTIT3_ITLine, ENABLE);
        /* Exit the while loop */
        return;
      }
    }
    /* Reinitialize PlayIntroDelay to 2sec */
    PlayIntroDelay = 20;
    /* Clear the LCD screen */
    LCD_Clear();
    /* Display "         To Exit " message */
    LCD_DisplayString(Line1, "         To Exit ", BlackText);
    /* Display "         Sub-Menu" message */
    LCD_DisplayString(Line2, "         Sub-Menu", BlackText);
    /* Display UPDOWNHelp graphic */
    LCD_DrawMasterGraphic(UPDOWNHelp);
    /* Wait 2sec or return if any pushbutton is pressed */
    while(PlayIntroDelay != 0)
    {
      /* If any key is pressed */
      if(JoyState()  != NOKEY)
      {
        /* Display "Help" menu */
        DisplayMenu();
        VIC_ITCmd(EXTIT3_ITLine, ENABLE);
        /* Exit the while loop */
        return;
      }
    }
    /* Reinitialize PlayIntroDelay to 2sec */
    PlayIntroDelay = 20;

    /* Clear the LCD screen */
    LCD_Clear();
    /* Display "          Scroll " message */
    LCD_DisplayString(Line1, "          Scroll ", BlackText);
    /* Display "         Sub-Menu" message */
    LCD_DisplayString(Line2, "         Sub-Menu", BlackText);
    /* Display LEFTRIGHTHelp graphic */
    LCD_DrawMasterGraphic(LEFTRIGHTHelp);
    /* Wait 2sec or return if any pushbutton is pressed */
    while(PlayIntroDelay != 0)
    {
      /* If any key is pressed */
      if(JoyState() != NOKEY)
      {
        /* Display "Help" menu */
        DisplayMenu();
        VIC_ITCmd(EXTIT3_ITLine, ENABLE);
        /* Exit the while loop */
        return;
      }
    }
    /* Reinitialize PlayIntroDelay to 2sec */
    PlayIntroDelay = 20;
    /* read the pressed key */
    MyKey = JoyState();
  }
  VIC_ITCmd(EXTIT3_ITLine, ENABLE);
}

/******************* (C) COPYRIGHT 2006 STMicroelectronics *****END OF FILE****/
