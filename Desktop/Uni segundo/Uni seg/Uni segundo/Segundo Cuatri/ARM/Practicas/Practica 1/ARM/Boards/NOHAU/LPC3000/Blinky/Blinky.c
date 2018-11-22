/******************************************************************************/
/* Blinky.c: LED Flasher                                                      */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2005-2006 Keil Software. All rights reserved.                */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/


#include "LPC318x.h"                    /* LPC3000 definitions                */
#include "GLCD.h"                       /* Graphic display function prototypes*/

#define LED_NUM               2         /* Number of LEDs                     */

/* LED constants                                                              */
const int led_mask[] = { (1<<9), (1<<10) };

/******************************************************************************
* Function: Wait for a number of cycles                                       *
*******************************************************************************/
 
void wait (void) {
  int delay = 10000000;

  while (delay--);
}


/******************************************************************************
* Main Program                                                                *
*******************************************************************************/

int main (void)  {
  int i;

  lcd_init();
  lcd_clear();
  lcd_print(" This is a Keil ", 0);
  lcd_print("  DEMO for the  ", 1);
  lcd_print("Philips LPC3000 ", 2);
  lcd_print("                ", 3);
  lcd_print("  www.keil.com  ", 4);

  for (;;)  {                           /* Loop forever                       */
    for (i = 0; i < LED_NUM; i++) {
      PIO_OUTP_SET = led_mask[i];
      wait();
      PIO_OUTP_CLR = led_mask[i];
      wait();
    }
    for (i = (LED_NUM - 1); i >= 0; i--)  {
      PIO_OUTP_SET = led_mask[i];
      wait();
      PIO_OUTP_CLR = led_mask[i];
      wait();
    }
  }
}
