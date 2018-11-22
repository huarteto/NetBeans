/******************************************************************************/
/* BLINKY.C: LED Flasher                                                      */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2005-2007 Keil Software. All rights reserved.                */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/

#include "S3C2440.h"

#define LED_NUM     3

/* LEDs are on GPC port                                                       */
const int led_mask[] = { (1 << 0), (1 << 5), (1 << 6) };

extern volatile          char Speed;
extern volatile unsigned char Clock;


/* Wait Function                                                              */
void wait (void) {

  while (!Clock);
  Clock = 0;
}


int main(void) {

  int num = -1; 
  int dir =  1;

  TCFG0     =  99;                      /* Prescaler for Timer 0              */
  TCFG1     =  2;                       /* Timer 0 clock = PCLK / 8           */
  TCNTB0    =  624;                     /* Timer reload value = 10 ms         */
  TCMPB0    =  625;                     /* For PWM, not used                  */
  TCON     |=  (1 << 1);                /* Manually update counter values     */
  TCON     &= ~(1 << 1);                /* Disable manual update              */
  TCON     |=  (1 << 3) | (1 << 0);     /* Start Timer0 with reload enabled   */
 
  INTMSK   &= ~(BIT_TIMER0);            /* Enable timer 0 interrupt           */

	GPCDAT   |= 0x61;                     /* Turn off all LEDs                  */

  for (;;) {                            /* Loop forever                       */

    /* Calculate 'num': 0, 1, ... , LED_NUM-1, LED_NUM-1, ... , 1, 0, 0, ...  */
    num += dir;
    if (num == LED_NUM) { dir = -1; num =  LED_NUM-1; } 
    else if   (num < 0) { dir =  1; num =  0;         }

    /* Turn ON and OFF LED with index 'num'                                   */
    GPCDAT &= ~led_mask[num];           /* Turn LED on                        */
    wait();
    GPCDAT |=  led_mask[num];           /* Turn LED off                       */
    wait();
  }
}
