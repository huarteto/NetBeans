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
#include "LCD.h"                        /* LCD function prototypes            */
#include <stdio.h>

/* Import functions from Setup.c                                              */
extern void SetupClock  (void);
extern void SetupADC    (void);
extern void SetupLED    (void);
extern void SetupUART   (void);

extern unsigned short int ADC_ConvertedValue;

extern unsigned char Clock1s;

int AD_DbgVal;

int main (void) {
  int i;
  int AD_value;
  int AD_scaled_ex = -1;
  int AD_scaled;

  SetupClock();
  SetupADC  ();
  SetupLED  ();
  SetupUART ();

  lcd_init  ();
  lcd_clear ();
  lcd_print (" MCBSTM32 DEMO  ");
  set_cursor(0, 1);
  lcd_print ("  www.keil.com  ");

  for (i = 0; i < 10000000; i++);       /* Wait for initial display           */

  while (1) {                           /* Loop forever                       */
    AD_value  = ADC_ConvertedValue;     /* Read AD value                      */
    AD_scaled = AD_value / 52;          /* AD value scaled to 0 .. 78         */
    if (AD_scaled != AD_scaled_ex) {    /* If new AD value different than old */
      AD_scaled_ex = AD_scaled;
      AD_DbgVal    = AD_value;
      lcd_bargraph(0, 1, AD_scaled);    /* Display bargraph according to AD   */
    }

    /* Printf message with AD value to serial port every 1 second             */
    if (Clock1s) {
      Clock1s = 0;
      printf("AD value = 0x%04X\r\n", AD_value);
    }
  }
}
