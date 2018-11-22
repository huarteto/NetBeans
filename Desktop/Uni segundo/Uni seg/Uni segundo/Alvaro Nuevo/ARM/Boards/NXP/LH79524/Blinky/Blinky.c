/******************************************************************************/
/* BLINKY.C: LED Flasher                                                      */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2005-2007 Keil Software. All rights reserved.                */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/
                  
#include <LH79524.H>                    /* NXP LH79524 and LH79525 registers  */

#define LED_NUM     2                   /* Number of user LEDs                */
#define LED0        (1 << 1)            /* LED STATUS0                        */
#define LED1        (1 << 2)            /* LED STATUS1                        */
#define LED_MSK     (LED0 | LED1)       /* Mask of all LEDs                   */

const int led_mask[] = {LED0, LED1};    /* Status LEDs                        */

extern volatile unsigned char Clock;

extern __irq void TIMER0_IRQ_Handler (void);


void wait (void) {                      /* Wait Function                      */

  while (!Clock);
  Clock = 0;
}


int main (void) {                       /* Main Function                      */

  int num      = -1; 
  int dir      =  1;

  CPLD_LED |=  LED_MSK;                 /* Turn off all LEDs                  */

  /* Setup Timer0 to generate interrupt every 10 ms                           */
  TIMER0->cmp_cap_ctrl  = (1 << 14);    /* Clear counter upon T0CMP1 match    */
  TIMER0->int_ctrl      = (1 <<  2);    /* Enable T0CMP1 interrupt            */
  TIMER0->cmp1          = 3969;         /* Compare match at every 10 ms       */
  TIMER0->ctrl          = 0x001B;       /* Timer clock=HCLK/128, start, clear */

  /* Setup VIC for Timer0 interrupt                                           */
  VIC->vectaddr[0] = (unsigned long)TIMER0_IRQ_Handler;     /* Set int vector */
  VIC->vectcntl[0] = 0x20 | VIC_TIMER0; /* Enable Vector0 for Timer0 interrupt*/
  VIC->intenable   = 1 << VIC_TIMER0;   /* Enable Timer0 interrupt            */

  for (;;) {                            /* Loop forever                       */

    /* Calculate 'num': 0, 1, ... , LED_NUM-1, LED_NUM-1, ... , 1, 0, 0, ...  */
    num += dir;
    if (num == LED_NUM) { dir = -1; num =  LED_NUM-1; } 
    else if   (num < 0) { dir =  1; num =  0;         }

    /* Turn  on LED with index 'num'                                          */
    CPLD_LED &= ~led_mask[num];
    wait();
    /* Turn off all LEDs                                                      */
    CPLD_LED |=  LED_MSK;
    wait();
  }
}
