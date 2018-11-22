/******************************************************************************/
/* IRQ.c: IRQ Handlers                                                        */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2007-2009 Keil Software. All rights reserved.                */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/

#include <LPC313x.H>


volatile unsigned long ticks = 0;
volatile unsigned char Clock;


/* Common interrupt handler, handles all interrupts                           */
__irq void IRQ_Handler (void) {

  switch ((INTC->irq_vec >> 3) & 0xFF) {
    case IRQ_TIMER3:                    /* Timer 3 interrupt index            */
      if (ticks++ >= (50-1)) {          /* Set Clock to 1 periodically        */
        ticks   = 0;
        Clock   = 1;
      }
      TIMER_CNTR3->clear = 0;           /* Clear Timer 3 interrupt status     */
      INTC->request[IRQ_TIMER3] = (1 << 29);
      break;
  }
}
