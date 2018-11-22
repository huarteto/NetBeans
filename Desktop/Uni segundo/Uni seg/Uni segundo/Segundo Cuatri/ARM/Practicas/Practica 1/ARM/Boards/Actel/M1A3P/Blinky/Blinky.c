/******************************************************************************/
/* BLINKY.C: LED Flasher                                                      */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2005-2008 Keil Software. All rights reserved.                */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/


// Sample Hardware definitions
#define GPIO_DATA (*((volatile unsigned long *)0xA0000000))


/*
 * Delay function
 */

void Delay (void) {
  int  i, j;

  while (!(GPIO_DATA & 1));	                    /* Wait while SW2 #1 OFF */
  j = (GPIO_DATA & 2) ? 100000 : 300000;	    /* Determine Speed: SW2 #2 */
  for (i = 0; i < j; i++);
}


/*
 * Main Program
 */

int main (void) {
  unsigned int n;

  while (1) {                                   /* Loop forever */
    /* Blink LEDs on Board */
    for (n = 1; n < 0x200; n <<= 1) {	 
	  GPIO_DATA = n;					        /* Output to LEDs */
      Delay();
    }
    for (n = 0x200; n > 1; n >>= 1) {
	  GPIO_DATA = n;					        /* Output to LEDs */
      Delay();
    }
  }
}
