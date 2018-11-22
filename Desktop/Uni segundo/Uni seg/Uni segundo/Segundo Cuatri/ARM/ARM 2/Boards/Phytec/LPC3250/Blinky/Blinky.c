/******************************************************************************/
/* Blinky.c: LED Flasher                                                      */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2005-2008 Keil Software. All rights reserved.                */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/


#include "LPC318x.h"                    /* LPC3000 definitions                */

#define LED_NUM               2         /* Number of LEDs                     */

/* LED constants                                                              */
const int led_mask[] = { (1<<1), (1<<14) };

/******************************************************************************
* Function: Wait for a number of cycles                                       *
*******************************************************************************/
 
void wait (unsigned int delay) {
  while (delay--);
}

/******************************************************************************
* Main Program                                                                *
*******************************************************************************/

int main (void)  {
  int i;

  PIO_OUTP_CLR = led_mask[0] | led_mask[1];

  for (;;)  {                           /* Loop forever                       */
    for (i = 0; i < LED_NUM; i++) {
      PIO_OUTP_SET = led_mask[i];
      wait(25000000);
      PIO_OUTP_CLR = led_mask[i];
      wait(25000000);
    }
    for (i = (LED_NUM - 1); i >= 0; i--)  {
      PIO_OUTP_SET = led_mask[i];
      wait(25000000);
      PIO_OUTP_CLR = led_mask[i];
      wait(25000000);
    }
  }
}
