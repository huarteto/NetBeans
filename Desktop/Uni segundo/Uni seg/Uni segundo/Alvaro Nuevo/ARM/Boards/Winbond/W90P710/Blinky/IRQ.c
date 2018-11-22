/******************************************************************************/
/* IRQ.C: IRQ Handlers                                                        */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2005-2007 Keil Software. All rights reserved.                */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/

#include <W90P710.H>                    /* Winbond W90P710 definitions        */


volatile unsigned long ticks = 0;
volatile unsigned char Clock;


/* Common interrupt handler, handles all interrupts                           */
__irq void IRQ_Handler (void) {

  switch (REG_AIC_ISNR & 0x1F) {
    case IRQ_TIMER1:                    /* Timer 0 interrupt index            */
      if (ticks++ >= (100-1)) {         /* Set Clock to 1 periodically        */
        ticks   = 0;
        Clock   = 1;
      }
      REG_TISR = 2;                     /* Clear Timer 1 interrupt status     */
      break;
  }
  REG_AIC_IPER;                         /* Acknowledge interrupt              */
  REG_AIC_EOSCR = 0;                    /* End of interrupt service routine   */
}
