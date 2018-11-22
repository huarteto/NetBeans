/******************************************************************************/
/* BLINKY.C: LED Flasher                                                      */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2005-2007 Keil Software. All rights reserved.                */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/

#include <stm32f10x_lib.h>              /* STM32F10x Library Definitions      */
#include "lcd.h"                        /* LCD function prototypes            */
#include <stdio.h>

/* Import functions from Setup.c                                              */
extern void SetupClock    (void);
extern void SetupADC      (void);
extern void SetupLED      (void);
extern void SetupUART     (void);
extern void SetupBacklight(void);

extern volatile unsigned short ADC_ConvertedValue;
extern volatile unsigned char  Clock1s;
extern volatile unsigned long  TimeTick;

#define AD_VALUE_MAX 0xFFF

/*------------------------------------------------------------------------------
  note: 
  LCD corrupts AD values due to HW design.
  set __USE_LCD in "options for target - C/C++ - Define" to enable Output on LCD
  set __USE_IRQ in "options for target - C/C++ - Define" to enable ADC in IRQ mode
                                                        default is ADC in DMA mode
 *------------------------------------------------------------------------------*/

int  AD_DbgVal;
char text[40];

/*------------------------------------------------------------------------------
  Additional LCD defines and functions used to draw a bargraph
 *------------------------------------------------------------------------------*/
#define FONT_WIDTH      16
#define FONT_HEIGHT     24
#define DISPLAY_WIDTH  320

static void drawPixel (unsigned short x, unsigned short y, unsigned short color) {
  LCD_SetCursor(x,y);
  LCD_WriteRAM_Prepare(); /* Prepare to write GRAM */
  LCD_WriteRAM(color);
}

static void drawBargraphH(unsigned short x, unsigned short y, unsigned short len, unsigned short color) {
  unsigned short i, j;

  for (i = 0; i < FONT_HEIGHT; i++) {
    for(j = 0; j < len; j++) {
	  drawPixel(x+i, DISPLAY_WIDTH-1-y-j, color);       
    }
  }
}


/*------------------------------------------------------------------------------
  Delay function
  delays number of tick Systicks (happens every 10 ms)
 *------------------------------------------------------------------------------*/
void Delay (unsigned long tick) {
  unsigned long timetick;

  timetick = TimeTick;
  while ((TimeTick - timetick) < tick);
}

int main (void) {
  int AD_value;
  int AD_valueOld = -1;
  int barLen;

  SetupClock();
  SetupADC  ();
  SetupLED  ();
  SetupUART ();
  SetupBacklight();


#ifdef __USE_LCD
  STM3210E_LCD_Init();                            // Initialize the LCD */
  LCD_Clear(White);                               // Clear the LCD */

  GPIO_WriteBit(GPIOA, GPIO_Pin_8, Bit_SET);      // Turn On LCD Backlight */

  LCD_SetBackColor(Blue);                         // Set the Back Color */
  LCD_SetTextColor(White);                        // Set the Text Color */

  LCD_DisplayStringLine(Line0, "   MCBSTM32E Demo   ");
  LCD_DisplayStringLine(Line1, "    STLIB Blinky    ");
  LCD_DisplayStringLine(Line2, "    www.keil.com    ");
  LCD_SetBackColor(White);                        // Set the Back Color */
  LCD_SetTextColor(Blue);                         // Set the Text Color */

  Delay(50);                                      // Wait for initial display
#endif // __USE_LCD

  while (1) {                                     // Loop forever
    AD_value  = ADC_ConvertedValue;               // Read AD value

    if (AD_value != AD_valueOld) {                // If new AD value different than old
      AD_DbgVal    = AD_value;
#ifdef __USE_LCD
	  barLen = (DISPLAY_WIDTH*AD_value)/AD_VALUE_MAX;
      drawBargraphH (Line6,      0,               barLen, Red  ); // draw new bar
      drawBargraphH (Line6, barLen, DISPLAY_WIDTH-barLen, White); // delete old bar
#endif // __USE_LCD

      AD_valueOld = AD_value;
    }

    /* Printf message with AD value to serial port every 1 second             */
    if (Clock1s) {
      Clock1s = 0;
      sprintf(text, "AD value = 0x%04X", AD_value);

      printf("%s\r\n", text);
#ifdef __USE_LCD
      LCD_DisplayStringLine(Line5, (unsigned char *)text);
#endif // __USE_LCD
    }
  }
}
