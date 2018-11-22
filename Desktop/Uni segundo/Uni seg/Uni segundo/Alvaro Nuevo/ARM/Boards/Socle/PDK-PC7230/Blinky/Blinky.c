/******************************************************************************/
/* Blinky.c: LED Flasher                                                      */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2005-2009 Keil Software. All rights reserved.                */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/


#include <PC72xx.H>                     /* Socle PC72xx register definitions  */

#define LED_NUM     1                   /* Number of user LEDs                */
#define LED_ENABLE  (*((volatile unsigned long *) 0x1801000C)) |=  0x80;
#define LED_ON(n)   (*((volatile unsigned long *) 0x18010008)) |=  0x80;
#define LED_OFF(n)  (*((volatile unsigned long *) 0x18010008)) &= ~0x80;

volatile unsigned long Clock;


__irq void IRQ_Handler (void) {         /* Common Interrupt Handler           */

  switch (*INTC0_ISR) {
    case SOCLE_INTC_TMR0_1:             /* Timer 1 interrupt index            */
      Clock          = 1;               /* Set to 1 every 1 ms                */
      *SOCLE_TMR1CON = 0x0182;          /* Timer 1 Module interrupt clear     */
      *INTC0_ICCR    = (1<<SOCLE_INTC_TMR0_1);    /* Interrupt Module int clr */
      break;
  }
}

void Delay (unsigned long ms) {         /* Delay Function (delay in ms)       */
  while (ms) {
    if (Clock) {
      Clock = 0;
      ms --;
    }
  }
}

int main (void) {                       /* Main Program                       */
  int idx = -1, dir = 1;

  LED_ENABLE                            /* Enable LED                         */
  LED_OFF(0)                            /* Turn LED off                       */

  /* Setup Timer 1 to generate interrupt every 1 ms                           */
  *SOCLE_TMR1LR  = (133000000UL/1000)/2;/* 1 ms at 133 MHz                    */
  *SOCLE_TMR1CON = 0x0182;              /* Enable timer, periodical, int en   */
  *INTC0_IMR  |= (1<<SOCLE_INTC_TMR0_1);/* Unmask Timer 1 interrupt           */
  *INTC0_IECR |= (1<<SOCLE_INTC_TMR0_1);/* Enable Timer 1 interrupt           */

  for (;;) {                            /* Loop forever                       */
    /* Calculate 'idx': 0,1,...,LED_NUM-1,LED_NUM-1,...,1,0,0,...             */
    idx += dir;
    if (idx == LED_NUM) { dir = -1; idx =  LED_NUM-1; } 
    else if   (idx < 0) { dir =  1; idx =  0;         }

    LED_ON(idx)                         /* Turn on LED 'idx'                  */
    Delay(100);                         /* Delay 100ms                        */

    LED_OFF(idx)                        /* Turn off LED 'idx'                 */
    Delay(900);                         /* Delay 900ms                        */
  }
}
