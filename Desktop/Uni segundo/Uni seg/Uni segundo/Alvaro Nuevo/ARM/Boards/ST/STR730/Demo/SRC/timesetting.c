/******************** (C) COPYRIGHT 2005 STMicroelectronics ********************
* File Name          : timesetting.c
* Author             : MCD Application Team
* Date First Issued  : 09/27/2005 :  V1.0
* Description        : This file provides functions to manage time, date and 
*                      alarm event.
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
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
struct time_t  {
  u8 sec_l;   
  u8 sec_h;
  u8 min_l;   
  u8 min_h;
  u8 hour_l;  
  u8 hour_h;
} ;
struct time_t time_struct;
struct alarm_t  {
  u8 sec_l;   
  u8 sec_h;
  u8 min_l;   
  u8 min_h;
  u8 hour_l;  
  u8 hour_h;
} ;
struct alarm_t alarm_struct;

struct date_t{
  u8 month;    
  u8 day; 
  u16 year;
} ;
struct date_t date_s;

bool FirstAlarmAdjust = TRUE;

RTC_InitTypeDef  RTC_InitStructure;
WDG_InitTypeDef  WDG_InitStructure;

/* Private function prototypes -----------------------------------------------*/
/* Interface functions -------------------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/*******************************************************************************
* Function Name  : ReadDigit 
* Description    : Reads digit entered by user, using demoboard keys.
* Input          : - ColBegin: begin column
*                : - CountBegin: begin counter
*                : - ValueMax: value max
*                : - ValueMin: value min
* Output         : None
* Return         : Digit value
*******************************************************************************/
u8 ReadDigit(u8 ColBegin, u8 CountBegin, u8 ValueMax, u8 ValueMin)
{
  u8 tmpValue = 0;
  u8 MyKey = 0;

  LCD_SetPosCur(2, ColBegin);
  LCD_SendCommand(LCD_CMD_DISPON_CURSON_BLKON);

  MyKey = ReadKey();

  tmpValue = CountBegin;
 
  while(TRUE)
  {	
    /* test on the value of the pressed key */
    if(MyKey == NXT)
    {
      /* increase the value of the digit */
      if(tmpValue == ValueMax)
      {
        tmpValue = (ValueMin - 1);
      }
      
      LCD_DigitDisplay(++tmpValue, 2, ColBegin);
      LCD_SetPosCur(2, ColBegin);
    }
    
    if(MyKey == PRV)
    {
      /* decrease the value of the digit */
      if(tmpValue == ValueMin)
      {
        tmpValue = (ValueMax + 1);
      }
      LCD_DigitDisplay(--tmpValue, 2, ColBegin);
      LCD_SetPosCur(2, ColBegin);
    }
    
    if(MyKey == SEL)
    {
      /* return the digit value and exit */
      return tmpValue;
    }
    MyKey = ReadKey();
  }
}

/*******************************************************************************
* Function Name  : Time_Init
* Description    : Initializes time application. 
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void Time_Init(void)
{
  date_s.month = 01;
  date_s.day = 01;
  date_s.year = 2005;
    	
  RTC_SetPrescaler(0x1E8);
  RTC_ITConfig(RTC_IT_ALA | RTC_IT_SEC, ENABLE); /* interrupt each second */

  EIC_IRQChannelConfig(RTC_IRQChannel , ENABLE);
  EIC_IRQChannelPriorityConfig(RTC_IRQChannel, 6);
}

/*******************************************************************************
* Function Name  : Time_Regulate 
* Description    : Returns the time entered by user, using demoboard keys.  
* Input          : None
* Output         : None
* Return         : Time value to be loaded in RTC counter register .
*******************************************************************************/
u32 Time_Regulate(void) 
{
  u8 Tmp_HH = 0, Tmp_MM = 0, Tmp_SS = 0;

  /* read hours */
  Tmp_HH = ReadDigit(5,time_struct.hour_h,2, 0x0);
       
  if(Tmp_HH == 2)
  {
    if(time_struct.hour_l > 3)
    {
      time_struct.hour_l = 0;
    }
    Tmp_HH = Tmp_HH*10 + ReadDigit(6,time_struct.hour_l,3, 0x0);      
  }
  else
  {
    Tmp_HH = Tmp_HH*10 + ReadDigit(6,time_struct.hour_l,9, 0x0);
  }

  /* read minutes */
  Tmp_MM = ReadDigit(8,time_struct.min_h,5, 0x0);
  Tmp_MM = Tmp_MM*10 + ReadDigit(9,time_struct.min_l,9, 0x0);

  /* read seconds */
  Tmp_SS = ReadDigit(11,time_struct.sec_h,5, 0x0);
  Tmp_SS = Tmp_SS*10 + ReadDigit(12,time_struct.sec_l,9, 0x0);

  LCD_SendCommand(LCD_CMD_DISPON_CURSOFF_BLKOFF);  
 
  return((Tmp_HH*3600 + Tmp_MM*60 + Tmp_SS));
}

/*******************************************************************************
* Function Name  : Time_Adjust
* Description    : Adjusts the current time (HH/MM/SS).
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void Time_Adjust(void) 
{  
  LCD_SendString("      :  :      ", 2, 1);
  
  /* display the current time */
  Time_Display(RTC_GetCounterValue(), 5, 2);
  
  /* disable the menu navigation keys */
  IntExtOnOffConfig(DISABLE);
  
  /* change the current time */
  RTC_SetCounter(Time_Regulate());
  
  /* enable the menu navigation keys */
  IntExtOnOffConfig(ENABLE);
  
}

/*******************************************************************************
* Function Name  : Time_Display
* Description    : Displays the current time.
* Input          : - TimeVar: time value.
*                : - Col: specifies the start column from where the time will 
*                    be displayed.
*                : - Row: specifies the line where the time will be displayed.
* Output         : None
* Return         : None
*******************************************************************************/
void Time_Display(u32 TimeVar, u8 Col , u8 Row)
{
  /* display hours */
  time_struct.hour_h=(u8)( TimeVar / 3600)/10;
  LCD_DigitDisplay(time_struct.hour_h, Row, Col);

  time_struct.hour_l=(u8)(((TimeVar)/3600)%10);
  LCD_DigitDisplay(time_struct.hour_l, Row, Col+1);
  
  /* display minutes */
  time_struct.min_h=(u8)(((TimeVar)%3600)/60)/10;
  LCD_DigitDisplay(time_struct.min_h, Row, Col+3);

  time_struct.min_l=(u8)(((TimeVar)%3600)/60)%10;
  LCD_DigitDisplay(time_struct.min_l, Row, Col+4);
  
  /* display seconds */
  time_struct.sec_h=(u8)(((TimeVar)%3600)%60)/10;
  LCD_DigitDisplay(time_struct.sec_h, Row, Col+6);

  time_struct.sec_l=(u8)(((TimeVar)%3600)%60)%10;
  LCD_DigitDisplay(time_struct.sec_l, Row, Col+7);
}

/*******************************************************************************
* Function Name  : Time_Show
* Description    : Shows the current time (HH/MM/SS) on LCD.
* Input          : None
* Output         : None
* Return         : None
******************************************************************************/
void Time_Show(void)
{
  while((ReadKey() != BACK))
  {
    Time_Display(RTC_GetCounterValue(), 5, 2);
  }  
}

/*******************************************************************************
* Function Name  : Date_Adjust
* Description    : Adjusts the current date (MM/DD/YYYY).
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void Date_Adjust(void) 
{   
  LCD_SendString("               ", 2, 1);
  
  /* display the current date */
  Date_Display(4,2,1);
  
  /* disable the menu navigation keys */
  IntExtOnOffConfig(DISABLE);
  
  /* change the current date */
  Date_Regulate();
  
  /* enable the menu navigation keys */
  IntExtOnOffConfig(ENABLE);
   
}

/*******************************************************************************
* Function Name  : Date_Regulate
* Description    : Sets the date entered by user, using demoboard keys. 
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void Date_Regulate(void)
{
  u8 Tmp_Month = 0, Tmp_Day = 0, tmp = 0;
  u16 Tmp_Year = 0;
  
  Tmp_Month = ReadDigit(4,date_s.month/10,1, 0x0);
  
  tmp = date_s.month%10;

  if(Tmp_Month == 1)
  {
    if(tmp > 2)
    {
      tmp = 0;
    }
    Tmp_Month = Tmp_Month*10 + ReadDigit(5,tmp,2, 0x0);
  }
  else
  {
    Tmp_Month = Tmp_Month*10 + ReadDigit(5,tmp,9, 0x1);
  }

  if(Tmp_Month == 2)
  {
    Tmp_Day = ReadDigit(7,date_s.day/10,2,0x0);
  }
  else
  {
    Tmp_Day = ReadDigit(7,date_s.day/10,3,0x0);
  }

  if(Tmp_Day == 3)
  {
    tmp = date_s.day%10;
    if(tmp >=1)
    {
      tmp = 0;
    }
    
    if(Tmp_Month==1 || Tmp_Month==3 || Tmp_Month==5 || Tmp_Month==7 ||
       Tmp_Month==8 || Tmp_Month==10 || Tmp_Month==12)
    {

      Tmp_Day = Tmp_Day*10 + ReadDigit(8,tmp,1, 0x0);
    }
    else
    {
      Tmp_Day = Tmp_Day*10 + ReadDigit(8,tmp,0, 0x0);
    }
  }
  else
  {
    if(Tmp_Day == 0)
    {
      Tmp_Day = Tmp_Day*10 + ReadDigit(8,date_s.day%10,9, 0x1);
    }
    else
    {
      Tmp_Day = Tmp_Day*10 + ReadDigit(8,date_s.day%10,9, 0x0);
    }
  }


  Tmp_Year = 1000 * ReadDigit(10,date_s.year/1000,9, 0x0);

  Tmp_Year =Tmp_Year + (ReadDigit(11,(date_s.year%1000)/100,9, 0x0)*100);

  Tmp_Year =Tmp_Year + (ReadDigit(12,((date_s.year%1000)%100)/10,9, 0x0)*10);

  Tmp_Year =Tmp_Year + ReadDigit(13,((date_s.year%1000)%100)%10,9, 0x0);

  if(Tmp_Year  == 0)
  {
    LCD_SendString("1", 2, 13);
  }
  
  if (Tmp_Month==2 && Tmp_Day==29 && (Tmp_Year % 4)!=0)
  {
    LCD_SendString("8", 2, 8);
    Tmp_Day=28;
  }
  
  LCD_SendCommand( LCD_CMD_DISPON_CURSOFF_BLKOFF);

  date_s.year = Tmp_Year;
  date_s.month = Tmp_Month;
  date_s.day = Tmp_Day;
}

/*******************************************************************************
* Function Name  : Date_Display
* Description    : Displays the current date.
* Input          : - Col: specifies the start column from where the date will 
*                    be displayed.
*                : - Row: specifies the line where the date will be displayed.
*                : - Type: specifies the date format. If Type = 1 then show
*                    (MM/DD/YYYY) else (MM/DD/YY).
* Output         : None
* Return         : None
*******************************************************************************/
void Date_Display(u8 Col, u8 Row, u8 Type)
{    
  LCD_DigitDisplay(date_s.month/10, Row, Col);
  LCD_DigitDisplay(date_s.month%10, Row, Col+1);
  LCD_SendString("/", Row, Col+2);
  LCD_DigitDisplay(date_s.day /10, Row, Col+3);
  LCD_DigitDisplay(date_s.day %10, Row, Col+4);
  LCD_SendString("/", Row, Col+5);
  
  if(Type == 1)
  {
    LCD_DigitDisplay(date_s.year/1000, Row, Col+6);
    LCD_DigitDisplay((date_s.year%1000)/100, Row, Col+7);
    LCD_DigitDisplay(((date_s.year%1000)%100)/10, Row, Col+8);
    LCD_DigitDisplay(((date_s.year%1000)%10)%10, Row, Col+9);
  }
  else
  {
    LCD_DigitDisplay(((date_s.year%1000)%100)/10, Row, Col+6);
    LCD_DigitDisplay(((date_s.year%1000)%10)%10, Row, Col+7);
  }
}

/*******************************************************************************
* Function Name  : Date_Show
* Description    : Shows date (MM/DD/YYYY) on LCD.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void Date_Show (void)
{
  while(ReadKey() != BACK)
  {
    Date_Display(4,2,1);
  }
}

/*******************************************************************************
* Function Name  : Date_Update
* Description    : Updates date when time is 23:59:59.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void Date_Update(void)
{
  if(date_s.month ==1 || date_s.month ==3 || date_s.month ==5 || date_s.month ==7 ||
     date_s.month ==8 || date_s.month ==10 || date_s.month ==12)
  {
    if(date_s.day < 31)
    {
      date_s.day++;
    }
    else /* date_s.day = 31 */
    {
      if(date_s.month != 12)
      {
        date_s.month ++;
        date_s.day = 1;
      }
      else /* date_s.day = 31 & date_s.month =12 */
      {  
        date_s.month = 1;
        date_s.day = 1;
        date_s.year++;
      }
    }      
  }
  else if(date_s.month == 4 || date_s.month == 6 || date_s.month == 9 ||
          date_s.month == 11)
  {
    if(date_s.day < 30)
    {
      date_s.day++;
    }
    else /* date_s.day = 30 */
    {
      date_s.month++;
      date_s.day = 1;
    }
  }  
  else if(date_s.month == 2)
  {
    if(date_s.day < 28)
    {
      date_s.day++;
    }
    else if(date_s.day == 28)
    {
      if(date_s.year%4 == 0) /* leap year */
      {
        date_s.day++;
      }
      else
      {
        date_s.month++;
        date_s.day = 1;
      }
    }
    else if(date_s.day == 29)
    {
      date_s.month++;
      date_s.day = 1;
    }
  }
}

/*******************************************************************************
* Function Name  : Alarm_Adjust
* Description    : Configures an alarm event to occurs within the current day.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void Alarm_Adjust(void)
{ 
  /* disable the menu navigation keys */  
  IntExtOnOffConfig(DISABLE);
  
  if(FirstAlarmAdjust == FALSE) /* an alarm event is already configured */
  {
    LCD_SendString("      :  :      ", 2, 1);
    
    /* display the alarm time */
    Alarm_Display(RTC_GetAlarmValue(), 5, 2);
  }
  else /* no alarm event is configured */
  {
    alarm_struct.hour_h = 0; 
    alarm_struct.hour_l = 0;
    alarm_struct.min_h = 0;
    alarm_struct.min_l = 0;
    alarm_struct.sec_h = 0;
    alarm_struct.sec_l = 0;    
    LCD_SendString("    00:00:00    ", 2, 1);
    FirstAlarmAdjust = FALSE;
  }
  
  /* change the alarm time */
  RTC_SetAlarm(Alarm_Regulate());
  
  /* enable the menu navigation keys */
  IntExtOnOffConfig(ENABLE);
}

/*******************************************************************************
* Function Name  : Alarm_Show
* Description    : Shows alarm time (HH/MM/SS) on LCD.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void Alarm_Show(void)
{
  while(( ReadKey() != BACK))
  {
      Alarm_Display(RTC_GetAlarmValue(), 5, 2);
  }
}

/*******************************************************************************
* Function Name  : Alarm_Regulate 
* Description    : Returns the alarm time entered by user, using demoboard keys. 
* Input          : None
* Output         : None
* Return         : Alarm time value to be loaded in RTC alarm register.
*******************************************************************************/
u32 Alarm_Regulate(void) 
{
  u8 Alarm_HH = 0, Alarm_MM = 0, Alarm_SS = 0;

  /* read hours */
  Alarm_HH = ReadDigit(5,alarm_struct.hour_h,2, 0x0);
       
  if(Alarm_HH == 2)
  {
    if(alarm_struct.hour_l > 3)
    {
      alarm_struct.hour_l = 0;
    }
    Alarm_HH = Alarm_HH*10 + ReadDigit(6,alarm_struct.hour_l,3, 0x0);       
  }
  else
  {
    Alarm_HH = Alarm_HH*10 + ReadDigit(6,alarm_struct.hour_l,9, 0x0);
  }
  
  /* read minutes */
  Alarm_MM = ReadDigit(8,alarm_struct.min_h,5, 0x0);
  Alarm_MM = Alarm_MM*10 + ReadDigit(9,alarm_struct.min_l,9, 0x0);

  /* read seconds */
  Alarm_SS = ReadDigit(11,alarm_struct.sec_h,5, 0x0);
  Alarm_SS = Alarm_SS*10 + ReadDigit(12,alarm_struct.sec_l,9, 0x0);

  LCD_SendCommand( LCD_CMD_DISPON_CURSOFF_BLKOFF);
  
  return((Alarm_HH*3600 + Alarm_MM*60 + Alarm_SS));
}

/*******************************************************************************
* Function Name  : Alarm_Display
* Description    : Displays alarm time.
* Input          : - AlarmVar: alarm time value.
*                : - Col: specifies the start column from where the alarm time  
*                    will be displayed.
*                : - Row: specifies the line where the alarm time will be 
*                    displayed.
* Output         : None
* Return         : None
*******************************************************************************/
void Alarm_Display(u32 AlarmVar, u8 Col , u8 Row)
{
  /* display hours */
  alarm_struct.hour_h=(u8)( AlarmVar / 3600)/10;
  LCD_DigitDisplay(alarm_struct.hour_h, Row, Col);

  alarm_struct.hour_l=(u8)(((AlarmVar)/3600)%10);
  LCD_DigitDisplay(alarm_struct.hour_l, Row, Col+1);
  
  /* display minutes */
  alarm_struct.min_h=(u8)(((AlarmVar)%3600)/60)/10;
  LCD_DigitDisplay(alarm_struct.min_h, Row, Col+3);

  alarm_struct.min_l=(u8)(((AlarmVar)%3600)/60)%10;
  LCD_DigitDisplay(alarm_struct.min_l, Row, Col+4);
  
  /* display seconds */
  alarm_struct.sec_h=(u8)(((AlarmVar)%3600)%60)/10;
  LCD_DigitDisplay(alarm_struct.sec_h, Row, Col+6);

  alarm_struct.sec_l=(u8)(((AlarmVar)%3600)%60)%10;
  LCD_DigitDisplay(alarm_struct.sec_l, Row, Col+7);
}

/*******************************************************************************
* Function Name  : ScreenSaver_Init
* Description    : Initializes the demonstration screen saver mode. The screen
*                  saver mode is entered if any of menu navigation keys are not
*                  pressed during 30 second and it is managed in the WDG interrupt
*                  handler routine.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void ScreenSaver_Init(void)
{
  /* configure the WDG to generate an interrupt each 30s */
  WDG_InitStructure.WDG_Mode = WDG_Mode_Timer;
  WDG_InitStructure.WDG_CLK_Source = WDG_CLK_EXTERNAL; /* EXTCLK := 31.25 KHz */
  WDG_InitStructure.WDG_Prescaler = 0xFF;
  WDG_InitStructure.WDG_Preload =  0xEA5;

  WDG_DeInit( );
  WDG_Init(&WDG_InitStructure);

  WDG_ITConfig(ENABLE);
  WDG_Cmd(ENABLE);

  EIC_IRQChannelConfig(WDGWUT_IRQChannel, ENABLE);
  EIC_IRQChannelPriorityConfig(WDGWUT_IRQChannel, 2);
}

/*******************************************************************************
* Function Name  : ScreenSaver_Show
* Description    : Shows time and date on LCD.
* Input          : None
* Output         : None
* Return         : None
******************************************************************************/
void ScreenSaver_Show(void)
{
  LCD_SendString("                ", 1, 1);
  LCD_SendString("      :  :      ", 2, 1);  
  
  /* disable screen saver mode */
  WDG_Cmd(DISABLE);
  
  /* disable the menu navigation keys */  
  IntExtOnOffConfig(DISABLE);  
  
  while(TRUE)
  {
    if(ReadKey() != NOKEY)
    {
      /* enable the menu navigation keys */
      IntExtOnOffConfig(ENABLE); 
      
      /* enable screen saver mode */
      WDG_Cmd(ENABLE);    	
      
      /* exit from screen saver mode */
      return;
    }
    /* display the current date */
    Date_Display(4,1,1);
    
    /* display the current time */
    Time_Display(RTC_GetCounterValue(), 5, 2); 	    
  }  
}

/******************* (C) COPYRIGHT 2005 STMicroelectronics *****END OF FILE****/
