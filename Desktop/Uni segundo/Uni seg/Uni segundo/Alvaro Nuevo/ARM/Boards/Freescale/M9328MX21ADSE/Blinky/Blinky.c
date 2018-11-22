/******************************************************************************/
/* Blinky.c: LED Flasher                                                      */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2005-2008 Keil Software. All rights reserved.                */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/


#include <iMX21.h>                      /* i.MX21 definitions                 */

#define M8(adr)  (*((volatile unsigned char  *) (adr)))
#define M16(adr) (*((volatile unsigned short *) (adr)))
#define M32(adr) (*((volatile unsigned long  *) (adr)))

#define LED_NUM     2                   /* Number of user LEDs                */
const int led_mask[] = {1<<15,1<<14};   /* LED4 and LED4                      */


void wait (unsigned int delay) {  
  while (delay--);  
}


int main (void)  {
  int num = -1;
  int dir =  1;
  
  for (;;) {                            /* Loop forever                       */
    /* Calculate 'num': 0, 1, ... , LED_NUM-1, LED_NUM-1, ... , 1, 0, 0, ...  */
    num += dir;
    if (num == LED_NUM) { dir = -1; num =  LED_NUM-1; } 
    else if   (num < 0) { dir =  1; num =  0;         }

    M16(0xCC800000) |=  led_mask[num];  /* Turn On  LED with index 'num'      */
    wait(5000000);
    M16(0xCC800000) &= ~led_mask[num];  /* Turn Off LED with index 'num'      */
    wait(5000000);
  }
}
