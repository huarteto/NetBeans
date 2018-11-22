/******************************************************************************/
/* BLINKY.C: LED Flasher                                                      */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2005-2007 Keil Software. All rights reserved.                */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/
                  
#include <LH7A404.H>                    /* NXP LH7A404 register definitions   */

#define LED_NUM     2                   /* Number of user LEDs                */
#define LED0        (1 << 6)            /* STATUS LED0                        */
#define LED1        (1 << 7)            /* STATUS LED1                        */
#define LED_MSK     (LED0 | LED1)       /* Mask of all LEDs                   */

const int led_mask[] = {LED0, LED1};    /* Status LEDs                        */

extern volatile unsigned char Clock;

extern __irq void TIMER1_IRQ_Handler (void);


void wait (void) {                      /* Wait Function                      */

  while (!Clock);
  Clock = 0;
}


int main (void) {                       /* Main Function                      */

  int num      = -1; 
  int dir      =  1;

  GPIO->peddr |=  LED_MSK;              /* Set LED pins as outputs            */
  GPIO->pedr  |=  LED_MSK;              /* Turn off all LEDs                  */

  /* Setup Timer1 to generate interrupt every 10 ms                           */
  TIMER1->control = (1 << 7) | (1 << 6);/* Enable, periodic mode, 2kHz clock  */
  TIMER1->load    = 2000 / 100;         /* Reload value 2 kHz / 100           */

  /* Setup VIC1 for Timer1 interrupt                                          */
  VIC1->intsel     &= ~VIC1_TC1OINTR;   /* Timer1 generates IRQ interrupt     */
  VIC1->vecaddrs[0] = (PFV)TIMER1_IRQ_Handler;  /* Set interrupt vector       */
  VIC1->vec_cntl[0] =  VIC_VEC_EN|VIC_TC1UINTR; /* Use Vector0 for Timer1 int */
  VIC1->inten      |=  VIC1_TC1OINTR;   /* Enable Timer1 interrupt            */

  for (;;) {                            /* Loop forever                       */

    /* Calculate 'num': 0, 1, ... , LED_NUM-1, LED_NUM-1, ... , 1, 0, 0, ...  */
    num += dir;
    if (num == LED_NUM) { dir = -1; num =  LED_NUM-1; } 
    else if   (num < 0) { dir =  1; num =  0;         }

    /* Turn  on LED with index 'num'                                          */
    GPIO->pedr &= ~led_mask[num];
    wait();
    /* Turn off all LEDs                                                      */
    GPIO->pedr |=  LED_MSK;
    wait();
  }
}
