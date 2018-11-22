/******************************************************************************/
/* BLINKY.C: LED Flasher                                                      */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2005-2007 Keil Software. All rights reserved.                */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/


#include "..\EB.h"                      /* Emulation Baseboard definitions */


/*
 * Delay function
 */

void Delay (void) {
  int i;

  for (i = 0; i < 1000000; i++);
}


/*
 * Main Program
 */

int main (void) {
  unsigned int n;

  while (1) {                           /* Loop forever */
    /* Blink User LEDs on Baseboard */
    for (n = 0x01; n < 0x80; n <<= 1) {
      HWR(BRD_BASE + BRD_LED) = n;      /* Output to LEDs */
      Delay();
    }
    for (n = 0x80; n > 0x01; n >>= 1) {
      HWR(BRD_BASE + BRD_LED) = n;      /* Output to LEDs */
      Delay();
    }
  }
}
