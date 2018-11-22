/******************************************************************************/
/* Blinky.c: LED Flasher                                                      */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2005-2009 Keil Software. All rights reserved.                */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/

#include <TMPA900.H>


 
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
  Timer5IntClr = 0;
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
  Timer5Load       = (48000000/256)/100;
  Timer5Control    = (1 << 7) | (1 << 6) | (1 << 5) | (2 << 2) | (1 << 1);

  for (;;)  {                           /* Loop forever                       */
    if (num++ >= 9) num = 0;

    
		GPIOBDATA	= num; /* Display number on 7-segment LED    */
    wait();
  }
}
