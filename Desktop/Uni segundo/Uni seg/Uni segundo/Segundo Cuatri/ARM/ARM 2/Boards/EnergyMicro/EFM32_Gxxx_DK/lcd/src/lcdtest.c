/**************************************************************************//**
 * @file
 * @brief LCD test/demo routines
 * @author Energy Micro AS
 * @version 1.2.0
 ******************************************************************************
 * @section License
 * <b>(C) Copyright 2009 Energy Micro AS, http://www.energymicro.com</b>
 ******************************************************************************
 *
 * This source code is the property of Energy Micro AS. The source and compiled
 * code may only be used on Energy Micro "EFM32" microcontrollers.
 *
 * This copyright notice may not be removed from the source code nor changed.
 *
 * DISCLAIMER OF WARRANTY/LIMITATION OF REMEDIES: Energy Micro AS has no
 * obligation to support this Software. Energy Micro AS is providing the
 * Software "AS IS", with no express or implied warranties of any kind,
 * including, but not limited to, any implied warranties of merchantability
 * or fitness for any particular purpose or warranties against infringement
 * of any proprietary rights of a third party.
 *
 * Energy Micro AS will not be liable for any consequential, incidental, or
 * special damages, or any other relief, or for any claim by any third party,
 * arising from your use of this Software.
 *
 *****************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "efm32.h"
#include "lcdcontroller.h"

/** SysTick polled Delay routine */
extern void Delay(uint32_t dlyTicks);

/** Demo scroll text */
static char *stext = "Welcome to Energy Micro EFM32        ";

/**************************************************************************//**
 * @brief LCD scrolls a text over the display, sort of "polled printf"
 * @param lcd Pointer to LCD register block
 *****************************************************************************/
void LCD_ScrollText(LCD_TypeDef *lcd, char *scrolltext)
{
  int  i, len;
  char buffer[8];

  buffer[7] = 0x00;
  len       = strlen(scrolltext);
  if (len < 7) return;
  for (i = 0; i < (len - 7); i++)
  {
    memcpy(buffer, scrolltext + i, 7);
    LCD_Write(lcd, buffer);
    Delay(125);
  }
}


/**************************************************************************//**
 * @brief LCD Blink Test
 * @param lcd Pointer to LCD register block
 *****************************************************************************/
void LCD_BlinkTest(LCD_TypeDef *lcd)
{
  LCD_EnergyMode(lcd, 0, 1);
  LCD_EnergyMode(lcd, 1, 1);
  LCD_EnergyMode(lcd, 2, 1);
  LCD_EnergyMode(lcd, 3, 1);
  LCD_EnergyMode(lcd, 4, 1);
  LCD_Number(lcd, 2359);
  LCD_Symbol(lcd, LCD_SYMBOL_COL10, 1);
  LCD_Symbol(lcd, LCD_SYMBOL_GECKO, 1);
  LCD_Symbol(lcd, LCD_SYMBOL_EFM32, 1);
  LCD_Write(lcd, " EFM32 ");
  lcd->BACTRL |= LCD_BACTRL_BLINKEN;
  while (lcd->SYNCBUSY) ;
  Delay(2000);
  LCD_EnergyMode(lcd, 4, 0);
  Delay(62);
  LCD_EnergyMode(lcd, 3, 0);
  Delay(62);
  LCD_EnergyMode(lcd, 2, 0);
  Delay(62);
  LCD_EnergyMode(lcd, 1, 0);
  Delay(62);
  LCD_EnergyMode(lcd, 0, 0);
  lcd->BACTRL &= ~LCD_BACTRL_BLINKEN;
  while (lcd->SYNCBUSY) ;
}

/**************************************************************************//**
 * @brief LCD Test Routine, shows various text and patterns
 *****************************************************************************/
void LCD_Test(LCD_TypeDef *lcd)
{
  int i, numberOfIterations = 1000;

  /* Exercise for the reader: Go to EM2 here and on all delays... */
  while (--numberOfIterations)
  {
    LCD_AllOff(lcd);
    for (i = 100; i > 0; i--)
    {
      LCD_Number(lcd, i);
      Delay(10);
    }
    LCD_NumberOff(lcd);

    LCD_Symbol(lcd, LCD_SYMBOL_GECKO, 1);
    LCD_Symbol(lcd, LCD_SYMBOL_EFM32, 1);
    LCD_Write(lcd, " Gecko ");
    Delay(1000);

    LCD_AllOn(lcd);
    Delay(1000);

    LCD_AllOff(lcd);
    LCD_Write(lcd, "OOOOOOO");
    Delay(62);
    LCD_Write(lcd, "XXXXXXX");
    Delay(62);
    LCD_Write(lcd, "+++++++");
    Delay(62);
    LCD_Write(lcd, "@@@@@@@");
    Delay(62);
    LCD_Write(lcd, "ENERGY ");
    Delay(250);
    LCD_Write(lcd, "@@ERGY ");
    Delay(62);
    LCD_Write(lcd, " @@RGY ");
    Delay(62);
    LCD_Write(lcd, " M@@GY ");
    Delay(62);
    LCD_Write(lcd, " MI@@Y ");
    Delay(62);
    LCD_Write(lcd, " MIC@@ ");
    Delay(62);
    LCD_Write(lcd, " MICR@@");
    Delay(62);
    LCD_Write(lcd, " MICRO@");
    Delay(62);
    LCD_Write(lcd, " MICRO ");
    Delay(250);
    LCD_Write(lcd, "-EFM32-");
    Delay(250);

    /* Various eye candy */
    LCD_AllOff(lcd);
    for (i = 0; i < 8; i++)
    {
      LCD_Number(lcd, numberOfIterations + i);
      LCD_ARing(lcd, i, 1);
      Delay(20);
    }
    for (i = 0; i < 8; i++)
    {
      LCD_Number(lcd, numberOfIterations + i);
      LCD_ARing(lcd, i, 0);
      Delay(100);
    }

    for (i = 0; i < 5; i++)
    {
      LCD_Number(lcd, numberOfIterations + i);
      LCD_Battery(lcd, i);
      LCD_EnergyMode(lcd, i, 1);
      Delay(100);
      LCD_EnergyMode(lcd, i, 0);
      Delay(100);
    }
    LCD_Symbol(lcd, LCD_SYMBOL_ANT, 1);
    for (i = 0; i < 4; i++)
    {
      LCD_EnergyMode(lcd, i, 1);
      Delay(100);
    }
    /* Scroll text */
    LCD_ScrollText(lcd, stext);

    /* Blink test*/
    LCD_BlinkTest(lcd); \
  }
  /* Segment bits */
  LCD_AllOff(lcd);
  LCD_Disable(lcd);
}

