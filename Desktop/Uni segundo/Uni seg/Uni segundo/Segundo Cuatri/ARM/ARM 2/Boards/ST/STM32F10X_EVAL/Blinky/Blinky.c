/******************************************************************************/
/* BLINKY.C: LED Flasher                                                      */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2005-2007 Keil Software. All rights reserved.                */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/

#include <string.h>
#include <stm32f10x_lib.h>              /* STM32F10x Library Definitions */
#include "lcd.h"
#include "spi_flash.h"

#define LED_On(led)     GPIOC->ODR |=  (led << 6)
#define LED_Off(led)    GPIOC->ODR &= ~(led << 6)

extern const unsigned char Keil_Logo[];

/* Import functions from Setup.c */
extern void SetupClock  (void);
extern void SetupLED    (void);
extern void SetupKBD    (void);


volatile unsigned long TimeTick;

/* SysTick interrupt happens every 10 ms */
void SysTick_Handler (void) {
  TimeTick++;
}

void Delay (unsigned long tick) {
  unsigned long timetick;

  timetick = TimeTick;
  while ((TimeTick - timetick) < tick);
}


int main (void) {
  unsigned long cnt;
  unsigned char buf[6];

  SetupClock();
  SetupLED  ();
  SetupKBD  ();

  SPI_FLASH_Init();                     /* Initialize the SPI FLASH driver */
  LCD_Init();                           /* Initialize the LCD */
  LCD_Clear(White);                     /* Clear the LCD */

  /* Display ST Logo (if exists on SPI Flash) */
  SPI_FLASH_BufferRead(buf, 0x0064FD00, 6);
  if (strncmp((const char *)buf, "BM\x42\x58\x02\x00", 6) == 0) {
    LCD_SetDisplayWindow(239, 319, 240, 320);
    LCD_DrawBMP(0x0064FD00);
    Delay(100);
  }
  
  /* Display STM32 Slide (if exists on SPI Flash) */
  SPI_FLASH_BufferRead(buf, 0x00414400, 6);
  if (strncmp((const char *)buf, "BM\x42\x58\x02\x00", 6) == 0) {
    LCD_SetDisplayWindow(239, 319, 240, 320);
    LCD_DrawBMP(0x00414400);
    Delay(200);  
  }

  /* Display Keil Logo */
  LCD_Clear(White);                     /* Clear the LCD */
  LCD_SetBackColor(White);              /* Set the Back Color */
  LCD_SetTextColor(0x0238);             /* Set the Text Color */

  LCD_SetDisplayWindow(144, 240, 48, 160);
  LCD_DrawMonoBMP((u32)Keil_Logo);
  Delay(200);

  LCD_SetDisplayWindow(239, 319, 240, 320);
  LCD_Clear(White);                     /* Clear the LCD */

  LCD_SetBackColor(Blue);               /* Set the Back Color */
  LCD_SetTextColor(White);              /* Set the Text Color */
  LCD_DisplayStringLine(Line0, "STM32 Demonstration ");
  LCD_SetBackColor(White);              /* Set the Back Color */
  LCD_SetTextColor(Blue);               /* Set the Text Color */
  LCD_DisplayStringLine(Line2, "Designed with RV-MDK");

  LCD_SetBackColor(White);              /* Set the Back Color */
  LCD_SetTextColor(Green);              /* Set the Text Color */

  LCD_DisplayStringLine(Line6, "   Keyboard Test:   ");

  cnt = 0;

  for (;;) {

    LED_Off(0x0F);                      /* Turn Off all LEDs */
    LED_On (1 << (cnt/40));             /* Turn On one LED */

    if ((cnt & 0x07) == 0) {
      LCD_SetBackColor(Cyan);
      LCD_DisplayChar(Line4, 319-2*cnt, ' ');
      LCD_SetBackColor(White);
    }
    if (++cnt == 160) {
      cnt = 0;
      LCD_ClearLine(Line4);
    }

    LCD_SetTextColor(Red);

    /* Read "Left" Key */
    if(!GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_1)) {
      LCD_DisplayChar(Line8, 319-136, 0x80+9);
    } else {
      LCD_DisplayChar(Line8, 319-136, 0x80+8);
    }

    /* Read "Right" Key */
    if(!GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_0)) {
      LCD_DisplayChar(Line8, 319-168, 0x80+11);
    } else {
      LCD_DisplayChar(Line8, 319-168, 0x80+10);
    }

    /* Read "Up" Key */
    if(!GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_8)) {
      LCD_DisplayChar(Line7, 319-152, 0x80+5);
    } else {
      LCD_DisplayChar(Line7, 319-152, 0x80+4);
    }

    /* Read "Down" Key */
    if(!GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_14)) {
      LCD_DisplayChar(Line9, 319-152, 0x80+7);
    } else {
      LCD_DisplayChar(Line9, 319-152, 0x80+6);
    }

    /* Read "Sel" Key */
    if(!GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_12)) {
      LCD_DisplayChar(Line8, 319-152, 0x80+1);
    } else {
      LCD_DisplayChar(Line8, 319-152, 0x80+0);
    }

    LCD_SetTextColor(Magenta);

    /* Read "Key" Key */
    if(!GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_9)) {
      LCD_DisplayChar(Line8, 319-206, 0x80+3);
    } else {
      LCD_DisplayChar(Line8, 319-206, 0x80+2);
    }

    Delay(5);
  }
}
