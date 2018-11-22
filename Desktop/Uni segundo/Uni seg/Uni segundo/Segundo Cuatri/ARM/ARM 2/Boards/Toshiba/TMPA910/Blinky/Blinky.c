/******************************************************************************/
/* Blinky.c: LED Flasher                                                      */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2005-2008 Keil Software. All rights reserved.                */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/


#include <TMPA910.H>                    /* TMPA910 definitions                */

char LED[10] = { 0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F };
 
volatile char Clock1s;
volatile long TimeTick;

void wait (void) {                      /* Wait 1 second                      */
  while (!Clock1s);
  Clock1s = 0;
}

__irq void TIM45_IRQ_Handler (void)  {  /* Timer4, 5 Interrupt handler        */
  if (TimeTick++ >= 99) {
    Clock1s  = 1;
    TimeTick = 0;
  }
  TIMER5INTCLR = 0;
  VICADDRESS   = 0;
}

int main (void)  {
  int num = -1;

  /* Setup GPIOB port as 3-state outputs for driving 7-segment LED            */
  GPIOBODE = 0;

  /* Setup and enable Timer5 for interrupt generation every 10 ms             */
  VICVECTPRIORITY4 = 0x0F;
  VICVECTADDR4     = ((unsigned long)TIM45_IRQ_Handler); 
  VICINTENABLE     = (1 << 4);
  TIMER5LOAD       = (48000000/256)/100;
  TIMER5CONTROL    = (1 << 7) | (1 << 6) | (1 << 5) | (2 << 2) | (1 << 1);

  for (;;)  {                           /* Loop forever                       */
    if (num++ >= 9) num = 0;
    GPIOBDATA  = ~LED[num];             /* Display number on 7-segment LED    */
    wait();
  }
}
