/******************************************************************************/
/* BLINKY.C: LED Flasher                                                      */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2005-2006 Keil Software. All rights reserved.                */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/

#include <91M55800.H>                      /* AT91M55800A Definitions */

void delay (void)  {                       /* Delay Function */
  unsigned long x;

  for (x = 0; x < 1000000; x++);           /* only to delay for LED flashes */
}

int main (void)  {

  PIOA_PER = 1;                            /* Enable PA0 */
  PIOA_OER = 1;                            /* Set PA0 as Output */

  while (1) {                              /* Loop forever */
    PIOA_CODR = 1;                         /* Turn LED On  (PA0 = 0) */
    delay();                               /* Delay */
    PIOA_SODR = 1;                         /* Turn LED Off (PA0 = 1) */
    delay();                               /* Delay */
  }
}
