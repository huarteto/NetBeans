/******************** (C) COPYRIGHT 2006 STMicroelectronics ********************
* File Name          : calendar.c
* Author             : MCD Application Team
* Date First Issued  : 05/18/2006 : Version 1.0
* Description        : This file includes the calendar driver of STR91x-EVAL.
********************************************************************************
* History:
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
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

  /* Time Structure definition */
  RTC_TIME Time;

  /* Alarm Structure definition */
  RTC_ALARM Alarm;

  /* Date Structure definition */
  RTC_DATE Date;

  u8 hour_h = 0;
  u8 hour_l = 0;
  u8 min_h = 0;
  u8 min_l = 0;
  u8 sec_h = 0;
  u8 sec_l = 0;
  u8 alarm_h = 0;
  u8 alarm_l = 0;

  /* Days names definition */
  u8* WeekDayNames[7] = {" SUN", " MON", " TUE", " WED", " THU", " FRI", " SAT"};
  /* Months names definition */
  u8 MonthNames[]= {'J', 'A', 'N', 'F', 'E', 'B', 'M', 'A', 'R', 'A', 'P', 'R',
                    'M', 'A', 'Y', 'J', 'U', 'N', 'J', 'U', 'L', 'A', 'U', 'G',
                    'S', 'E', 'P', 'O', 'C', 'T', 'N', 'O', 'V', 'D', 'E', 'C'};

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/*******************************************************************************
* Function Name  : ReadDigit
* Description    : Reads digit entered by user, using menu navigation keys.
* Input          : None
* Output         : None
* Return         : Digit value
*******************************************************************************/
u8 ReadDigit(u8 ColBegin, u8 CountBegin, u8 ValueMax, u8 ValueMin)
{
  u8 MyKey = 0, tmpValue = 0;

  /* Initialize tmpValue */
  tmpValue = CountBegin;
  /* Display  */
  LCD_DisplayChar(Line2, ColBegin, (tmpValue + 0x30), WhiteText);

  /* Endless loop */
  while(TRUE)
  {	
    /* Check which key is pressed */
    MyKey = ReadKey();

    /* If "UP" pushbutton is pressed */
    if(MyKey == UP)
    {
      /* Increase the value of the digit */
      if(tmpValue == ValueMax)
      {
        tmpValue = (ValueMin - 1);
      }
      /* Display new value */
      LCD_DisplayChar(Line2, ColBegin,((++tmpValue) + 0x30) , WhiteText);
    }
    /* If "DOWN" pushbutton is pressed */
    if(MyKey == DOWN)
    {
      /* Decrease the value of the digit */
      if(tmpValue == ValueMin)
      {
        tmpValue = (ValueMax + 1);
      }
      /* Display new value */
      LCD_DisplayChar(Line2, ColBegin,((--tmpValue) + 0x30) , WhiteText);
    }
    /* If "SEL" pushbutton is pressed */
    if(MyKey == SEL)
    {
      /* Display new value */
      LCD_DisplayChar(Line2, ColBegin, (tmpValue + 0x30) , BlackText);
      /* Return the digit value and exit */
      return tmpValue;
    }
  }
}

/*******************************************************************************
* Function Name  : Calendar_Init
* Description    : Initializes calendar application.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void Calendar_Init(void)
{
  /* Initialize Time structure */
  Time.hours = 0;
  Time.minutes = 0;
  Time.seconds = 0;
  Time.milliseconds = 0;

  /* Initialize Date structure */
  Date.day = 1 ;
  Date.weekday = 0;
  Date.month = 1;
  Date.year = 6;
  Date.century = 20;

  /* Initialize Alarm structure */
  Alarm.day = 1;
  Alarm.hours = 0;
  Alarm.minutes = 0;
  Alarm.seconds = 0;

  /* Verify if the RTC Time register values are in the range */
  RTC_GetTime(BCD, &Time);
  if((Time.hours > 0x24) || (Time.minutes > 0x59) || (Time.seconds > 0x59) || (Time.milliseconds > 0x999))
  {
     Time.hours = 0;
     Time.minutes = 0;
     Time.seconds = 0;
     Time.milliseconds = 0;
     RTC_SetTime(Time);
  }
  /* Verify if the RTC Date register values are in the range */
  RTC_GetDate(BCD, &Date);
  if((Date.day > 0x31) || (Date.month > 0x12) || (Date.year > 0x99) || (Date.century > 0x99))
  {
    Date.day = 1 ;
    Date.weekday = 0;
    Date.month = 1;
    Date.year = 6;
    Date.century = 20;
    RTC_SetDate(Date);
  }
  if((Date.day < 1) || (Date.month < 1))
  {
    Date.day = 1 ;
    Date.weekday = 1;
    Date.month = 1;
    Date.year = 6;
    Date.century = 20;
    RTC_SetDate(Date);
  }
  /* Verify if the RTC Alarm register values are in the range */
  RTC_GetAlarm(BCD, &Alarm);
  if((Alarm.hours > 0x24) || (Alarm.minutes > 0x59) || (Alarm.seconds > 0x59) || (Alarm.day > 0x31))
  {
     Alarm.hours = 0;
     Alarm.minutes = 0;
     Alarm.seconds = 0;
     Alarm.day = 1;
     RTC_SetAlarm(Alarm);
  }
  if(Alarm.day < 1)
  {
     Alarm.hours = 0;
     Alarm.minutes = 0;
     Alarm.seconds = 0;
     Alarm.day = 1;
     RTC_SetAlarm(Alarm);
  }

  /* Enable the RTC Alarm Interrupt */
  RTC_ITConfig(RTC_IT_Alarm, ENABLE);
}

/*******************************************************************************
* Function Name  : Time_Regulate
* Description    : Sets the time entered by user, using menu vavigation keys.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void Time_Regulate(void)
{
  u8 Tmp_HH = 0, Tmp_MM = 0, Tmp_SS = 0;

  /* Read time hours */
  Tmp_HH = ReadDigit(32, hour_h, 0x2, 0x0);
  if(Tmp_HH == 2)
  {
    if(hour_l > 3)
    {
      hour_l = 0;
    }
    Tmp_HH = Tmp_HH*10 + ReadDigit(40, hour_l, 0x3, 0x0);
  }
  else
  {
    Tmp_HH = Tmp_HH*10 + ReadDigit(40, hour_l, 0x9, 0x0);
  }
  /* Read time  minutes */
  Tmp_MM = ReadDigit(56, min_h, 0x5, 0x0);
  Tmp_MM = Tmp_MM*10 + ReadDigit(64, min_l, 0x9, 0x0);
  /* Read time seconds */
  Tmp_SS = ReadDigit(80, sec_h, 0x5, 0x0);
  Tmp_SS = Tmp_SS*10 + ReadDigit(88, sec_l, 0x9, 0x0);

  Time.hours = Tmp_HH;
  Time.minutes = Tmp_MM;
  Time.seconds = Tmp_SS;
  Time.milliseconds = 0;
  RTC_SetTime(Time);
}

/*******************************************************************************
* Function Name  : Time_Adjust
* Description    : Adjusts the time entered by user, using demoboard keys.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void Time_Adjust(void)
{
  /* Clear Line2 */
  LCD_ClearLine(Line2);
  /* Disable the EXTIT3 interrupt */
  VIC_ITCmd(EXTIT3_ITLine, DISABLE);
  /* Display time separators ":" on Line2 */
  LCD_DisplayChar(Line2, 48, ':', BlackText);
  LCD_DisplayChar(Line2, 72, ':', BlackText);

  /* Display the current time */
  Time_Display();

  /* Change the current time */
  Time_Regulate();

  /* Enable the EXTIT3 interrupt */
  VIC_ITCmd(EXTIT3_ITLine, ENABLE);
  /* Execute UpDownFunc function to display upper menu */
  UpDownFunc();
}

/*******************************************************************************
* Function Name  : Time_Display
* Description    : Displays the current time.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void Time_Display(void)
{
  RTC_GetTime(BCD, &Time);

  /* Display time hours */
  hour_h=(u8)(Time.hours/16);
  LCD_DisplayChar(Line2, 32,(hour_h + 0x30) , BlackText);
  hour_l=(u8)((Time.hours)%16);
  LCD_DisplayChar(Line2, 40,(hour_l + 0x30) , BlackText);

  /* Display time minutes */
  min_h=(u8)(Time.minutes/16);
  LCD_DisplayChar(Line2, 56,(min_h + 0x30) , BlackText);
  min_l=(u8)((Time.minutes)%16);
  LCD_DisplayChar(Line2, 64,(min_l + 0x30) , BlackText);

  /* Display time seconds */
  sec_h=(u8)((Time.seconds)/16);
  LCD_DisplayChar(Line2, 80,(sec_h + 0x30) , BlackText);
  sec_l=(u8)((Time.seconds)%16);
  LCD_DisplayChar(Line2, 88,(sec_l + 0x30) , BlackText);
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

  /* Disable the EXTIT3 interrupt */
  VIC_ITCmd(EXTIT3_ITLine, DISABLE);
  /* Wait a press on "UP" or "DOWN" pushbuttons */
  while((JoyState() & UP) != UP)
  {
    /* Display time separators ":" on Line2 */
    LCD_DisplayChar(Line2, 48, ':', BlackText);
    LCD_DisplayChar(Line2, 72, ':', BlackText);
    /* Display current time */
    Time_Display();
  }
  /* Enable the EXTIT3 interrupt */
  VIC_ITCmd(EXTIT3_ITLine, ENABLE);
}

/*******************************************************************************
* Function Name  : Date_Regulate
* Description    : Sets the date entered by user, using menu navigation keys.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void Date_Regulate(void)
{
  u8 Tmp_Day = 0, tmp = 0;
  u8 Tmp_Year = 0, Tmp_Century = 0;
  u8 MyKey = 0;

  LCD_DisplayChar(Line2, 35, MonthNames[(((Date.month) -1) * 3)], WhiteText);
  LCD_DisplayChar(Line2, 42, MonthNames[((((Date.month) -1) * 3) + 1)], WhiteText);
  LCD_DisplayChar(Line2, 49, MonthNames[((((Date.month) -1) * 3) + 2)], WhiteText);
  /* Read date months */
  while(ReadKey() != SEL)
  {	
    /* Check which key is pressed */
    MyKey = ReadKey();

    /* If "UP" pushbutton is pressed */
    if(MyKey == UP)
    {
       if(Date.month == 12)Date.month = 0;
       LCD_DisplayChar(Line2, 35, MonthNames[(((++Date.month) -1) * 3)], WhiteText);
       LCD_DisplayChar(Line2, 42, MonthNames[((((Date.month) -1) * 3) + 1)], WhiteText);
       LCD_DisplayChar(Line2, 49, MonthNames[((((Date.month) -1) * 3) + 2)], WhiteText);
       Date.weekday = DayOfWeek( (((Date.century/10)*1000) +  ((Date.century%10)*100)+ Date.year), Date.month, Date.day);
       LCD_DisplayString(Line2, WeekDayNames[Date.weekday], BlackText);
    }
    /* If "DOWN" pushbutton is pressed */
    if(MyKey == DOWN)
    {
      if(Date.month == 1)Date.month = 13;
      LCD_DisplayChar(Line2, 35, MonthNames[(((--Date.month) -1) * 3)], WhiteText);
      LCD_DisplayChar(Line2, 42, MonthNames[((((Date.month) -1) * 3) + 1)], WhiteText);
      LCD_DisplayChar(Line2, 49, MonthNames[((((Date.month) -1) * 3) + 2)], WhiteText);
      Date.weekday = DayOfWeek( (((Date.century/10)*1000) +  ((Date.century%10)*100)+ Date.year), Date.month, Date.day);
      LCD_DisplayString(Line2, WeekDayNames[Date.weekday], BlackText);
    }

  }
  LCD_DisplayChar(Line2, 35, MonthNames[(((Date.month) -1) * 3)], BlackText);
  LCD_DisplayChar(Line2, 42, MonthNames[((((Date.month) -1) * 3) + 1)], BlackText);
  LCD_DisplayChar(Line2, 49, MonthNames[((((Date.month) -1) * 3) + 2)], BlackText);

  /* Read date days */
  if(Date.month == 2)
  {
    Tmp_Day = ReadDigit(63, Date.day/16, 0x2, 0x0);
  }
  else
  {
    Tmp_Day = ReadDigit(63, Date.day/16, 0x3, 0x0);
  }
  if(Tmp_Day == 3)
  {
    tmp = Date.day % 16;

    if(tmp >= 1)
    {
      tmp = 0;
    }

    if(Date.month == 1 || Date.month == 3 || Date.month == 5 || Date.month == 7 ||
       Date.month == 8 || Date.month == 10 || Date.month == 12)
    {

      Tmp_Day = Tmp_Day*10 + ReadDigit(70, tmp, 0x1, 0x0);
    }
    else
    {
      Tmp_Day = Tmp_Day*10 + ReadDigit(70, tmp, 0x0, 0x0);
    }
  }
  else
  {
    if(Tmp_Day == 0)
    {
      Tmp_Day = Tmp_Day*10 + ReadDigit(70, Date.day%16, 0x9, 0x1);
    }
    else
    {
      Tmp_Day = Tmp_Day*10 + ReadDigit(70, Date.day%16, 0x9, 0x0);
    }
  }
  Date.weekday = DayOfWeek((((Date.century/10)*1000) +  ((Date.century%10)*100)+ Date.year), Date.month, Tmp_Day);
  LCD_DisplayString(Line2, WeekDayNames[Date.weekday], BlackText);
  /* Read date years */
  Tmp_Century = 10 * ReadDigit(84, Date.century/16, 0x9, 0x0);

  Tmp_Century = Tmp_Century + (ReadDigit(91, (Date.century%16), 0x9, 0x0));

  Tmp_Year = Tmp_Year + (ReadDigit(98,((Date.year/16)), 0x9, 0x0));

  Tmp_Year = Tmp_Year + ReadDigit(105,((Date.year%16)), 0x9, 0x0);
  if((Tmp_Year  == 0)&& (Tmp_Century == 0))
  {
    /* Set year to 2006 */
    Tmp_Year = 0x6;
    Tmp_Century = 20;
  }
  if (Date.month == 2 && Tmp_Day == 29 && (Tmp_Year % 4)!= 0)
  {
    Tmp_Day = 28;
  }
  Date.weekday = DayOfWeek((((Date.century/10)*1000) +  ((Date.century%10)*100)+ Date.year), Date.month, Tmp_Day);
  LCD_DisplayString(Line2, WeekDayNames[Date.weekday], BlackText);

  /* Return the date value to store in date structure */
  Date.century = Tmp_Century;
  Date.year = Tmp_Year;
  Date.day = Tmp_Day;
  RTC_SetDate(Date);
}

/*******************************************************************************
* Function Name  : Date_Adjust
* Description    : Adjusts the current date (DayName Month Day Year).
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void Date_Adjust(void)
{
  /* Clear Line2 */
  LCD_ClearLine(Line2);

  /* Disable the EXTIT3 interrupt */
  VIC_ITCmd(EXTIT3_ITLine, DISABLE);
  /* Display the current date */
  Date_Display();

  /* Change the current date */
  Date_Regulate();

  /* Enable the EXTIT3 interrupt */
  VIC_ITCmd(EXTIT3_ITLine, ENABLE);
  /* Execute UpDownFunc function to display upper menu */
  UpDownFunc();
}

/*******************************************************************************
* Function Name  : Date_Display
* Description    : Displays the current date.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void Date_Display(void)
{
  RTC_GetDate(BCD, &Date);
  LCD_DisplayString(Line2, WeekDayNames[Date.weekday], BlackText);
  LCD_DisplayChar(Line2, 35, MonthNames[(((Date.month) -1) * 3)], BlackText);
  LCD_DisplayChar(Line2, 42, MonthNames[((((Date.month) -1) * 3) + 1)], BlackText);
  LCD_DisplayChar(Line2, 49, MonthNames[((((Date.month) -1) * 3) + 2)], BlackText);
  LCD_DisplayChar(Line2, 63, ((Date.day/16) + 0x30), BlackText);
  LCD_DisplayChar(Line2, 70, ((Date.day%16) + 0x30), BlackText);
  LCD_DisplayChar(Line2, 84, ((Date.century/16) + 0x30), BlackText);
  LCD_DisplayChar(Line2, 91, (((Date.century)%16) + 0x30), BlackText);
  LCD_DisplayChar(Line2, 98, (((Date.year)/16) + 0x30), BlackText);
  LCD_DisplayChar(Line2, 105, (((Date.year)%16) + 0x30), BlackText);
}

/*******************************************************************************
* Function Name  : Date_Show
* Description    : Shows date (DayName Month Day Year) on LCD.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void Date_Show (void)
{
  /* Clear Line2 */
  LCD_ClearLine(Line2);

  /* Disable the EXTIT3 interrupt */
  VIC_ITCmd(EXTIT3_ITLine, DISABLE);
  /* Wait a press on "UP" or "DOWN" pushbuttons */
  while((JoyState() & UP) != UP)
  {
    Date_Display();
  }

  /* Enable the EXTIT3 interrupt */
  VIC_ITCmd(EXTIT3_ITLine, ENABLE);
}

/*******************************************************************************
* Function Name  : DayOfWeek
* Description    : Return the number of the day in the week.
* Input          : None
* Output         : None
* Return         : The number of the day
*******************************************************************************/
u16 DayOfWeek(u16 nYear, u16 nMonth, u16 nDay)
{
  u16 nDayOfWeek = 0;
  u16 pnDaysBeforeMonth[14] = {0, 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365};
	 	
  /* The day of Jan 1, nYear */
  nDayOfWeek = 6 + nYear % 7 + CountOfFeb29(nYear) % 7 + 14; /* + 14 : makes nDayOfWeek >= 0 */
	
  /* The day of nMonth 1, nYear */
  nDayOfWeek += pnDaysBeforeMonth[nMonth];
  if( nMonth > 2 && IsLeapYear(nYear))	
  {
    nDayOfWeek++;
  }

  /* The day of nMonth nDay, nYear */
  nDayOfWeek += nDay - 1;
  nDayOfWeek %= 7;
  /* Return the number of the day in the week */	
  return nDayOfWeek;
}

/*******************************************************************************
* Function Name  : CountOfFeb29
* Description    : Return february days number.
* Input          : None
* Output         : None
* Return         : February days number.
*******************************************************************************/
u16 CountOfFeb29(u16 nYear)
{
  u16 nCount = 0;

  if(nYear > 0)
  {
    nCount = 1;	/* Year 0 is a leap year */
    nYear--;    /* Year nYear is not in the period */
  }
  nCount += nYear / 4 - nYear / 100 + nYear / 400;
  return nCount;
}

/*******************************************************************************
* Function Name  : IsLeapYear
* Description    : Check whether the passed year is Leap or not.
* Input          : None
* Output         : None
* Return         : TRUE: leap year
*                  FALSE: not leap year
*******************************************************************************/
bool IsLeapYear(u16 nYear)
{
  if(nYear % 4 != 0) return FALSE;
  if(nYear % 100 != 0) return TRUE;
  return (bool)(nYear % 400 == 0);
}

/*******************************************************************************
* Function Name  : Alarm_Regulate
* Description    : Sets the alarm time entered by user, using demoboard keys.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void Alarm_Regulate(void)
{
  u8 Alarm_DAY = 0, Alarm_HH = 0, Alarm_MM = 0, Alarm_SS = 0;

  /* Read Alarm Day */
  Alarm_DAY = ReadDigit(24, alarm_h, 0x3, 0x0);
  if(Alarm_DAY == 3)
  {
    if(alarm_l > 1)
    {
      alarm_l = 0;
    }
    Alarm_DAY = Alarm_DAY*10 + ReadDigit(32, alarm_l, 0x1, 0x0);
  }
  else
  {
    Alarm_DAY = Alarm_DAY*10 + ReadDigit(32, alarm_l, 0x9, 0x0);
  }

  /* Read time hours */
  Alarm_HH = ReadDigit(48, hour_h, 0x2, 0x0);
  if(Alarm_HH == 2)
  {
    if(hour_l > 3)
    {
      hour_l = 0;
    }
    Alarm_HH = Alarm_HH*10 + ReadDigit(56, hour_l, 0x3, 0x0);
  }
  else
  {
    Alarm_HH = Alarm_HH*10 + ReadDigit(56, hour_l, 0x9, 0x0);
  }
  /* Read time  minutes */
  Alarm_MM = ReadDigit(72, min_h, 0x5, 0x0);
  Alarm_MM = Alarm_MM*10 + ReadDigit(80, min_l, 0x9, 0x0);
  /* Read time seconds */
  Alarm_SS = ReadDigit(96, sec_h, 0x5, 0x0);
  Alarm_SS = Alarm_SS*10 + ReadDigit(104, sec_l, 0x9, 0x0);

  Alarm.hours = Alarm_HH;
  Alarm.minutes = Alarm_MM;
  Alarm.seconds = Alarm_SS;
  Alarm.day = Alarm_DAY;
  RTC_SetAlarm(Alarm);
}

/*******************************************************************************
* Function Name  : Alarm_Adjust
* Description    : Adjusts the alarm time entered by user, using demoboard keys.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void Alarm_Adjust(void)
{
  /* Clear LCD Line2 */
  LCD_ClearLine(Line2);
  LCD_DisplayChar(Line2, 40, '/', BlackText);
  /* Display alarm separators ":" on Line2 */
  LCD_DisplayChar(Line2, 64, ':', BlackText);
  LCD_DisplayChar(Line2, 88, ':', BlackText);
  VIC_ITCmd(EXTIT3_ITLine, DISABLE);

  /* Display the alarm value */
  Alarm_Display();
  /* Store new alarm value */
  Alarm_Regulate();

  VIC_ITCmd(EXTIT3_ITLine, ENABLE);
  /* Execute UpDownFunc function to display upper menu */
  UpDownFunc();
}

/*******************************************************************************
* Function Name  : Alarm_Display
* Description    : Displays alarm time.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void Alarm_Display(void)
{
  RTC_GetAlarm(BCD, &Alarm);

  /* Display time hours */
  alarm_h = (u8)(Alarm.day/16);
  LCD_DisplayChar(Line2, 24,(alarm_h + 0x30) , BlackText);
  alarm_l = (u8)((Alarm.day)%16);
  LCD_DisplayChar(Line2, 32,(alarm_l + 0x30) , BlackText);

  /* Display time hours */
  hour_h=(u8)(Alarm.hours/16);
  LCD_DisplayChar(Line2, 48,(hour_h + 0x30) , BlackText);
  hour_l=(u8)((Alarm.hours)%16);
  LCD_DisplayChar(Line2, 56,(hour_l + 0x30) , BlackText);

  /* Display time minutes */
  min_h=(u8)(Alarm.minutes/16);
  LCD_DisplayChar(Line2, 72,(min_h + 0x30) , BlackText);
  min_l=(u8)((Alarm.minutes)%16);
  LCD_DisplayChar(Line2, 80,(min_l + 0x30) , BlackText);

  /* Display time seconds */
  sec_h=(u8)((Alarm.seconds)/16);
  LCD_DisplayChar(Line2, 96,(sec_h + 0x30) , BlackText);
  sec_l=(u8)((Alarm.seconds)%16);
  LCD_DisplayChar(Line2, 104,(sec_l + 0x30) , BlackText);
}

/*******************************************************************************
* Function Name  : Alarm_Show
* Description    : Shows alarm time (DAY/HH:MM:SS) on LCD.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void Alarm_Show(void)
{

  /* Disable the EXTIT3 interrupt */
  VIC_ITCmd(EXTIT3_ITLine, DISABLE);
  /* Wait a press on "UP" or "DOWN" pushbuttons */
  while((ReadKey() & UP) != UP)
  {
    LCD_DisplayChar(Line2, 40, '/', BlackText);
    /* Display alarm separators ":" on Line2 */
    LCD_DisplayChar(Line2, 64, ':', BlackText);
    LCD_DisplayChar(Line2, 88, ':', BlackText);
    /* Display actual alarm value */
    Alarm_Display();
  }
  /* Enable the EXTIT3 interrupt */
  VIC_ITCmd(EXTIT3_ITLine, ENABLE);
}

/******************* (C) COPYRIGHT 2006 STMicroelectronics *****END OF FILE****/
