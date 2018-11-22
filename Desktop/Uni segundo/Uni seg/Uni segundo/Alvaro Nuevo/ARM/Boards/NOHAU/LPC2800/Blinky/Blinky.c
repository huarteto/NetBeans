/******************************************************************************/
/* Blinky.c: LED Flasher                                                      */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2005-2007 Keil Software. All rights reserved.                */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/

#include "LPC288x.h"                    /* LPC288x definitions                */

#define LED_LOW                         MODE1_2|=(1<<1);MODE0_2&=~(1<<1);
#define LED_HIGH                        MODE1_2|=(1<<1);MODE0_2|= (1<<1);

/******************************************************************************
* Function: Wait for a number of cycles                                       *
*******************************************************************************/
 
void wait (unsigned long delay) {
  while (delay--);
}


/******************************************************************************
* Main Program                                                                *
*******************************************************************************/

int main (void)  {
  for (;;)  {                           /* Loop forever                       */
    LED_LOW
    wait(1000000);
    LED_HIGH
    wait(1000000);
  }
}
