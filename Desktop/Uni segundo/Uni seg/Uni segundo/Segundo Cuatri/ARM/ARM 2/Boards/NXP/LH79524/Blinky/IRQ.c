/******************************************************************************/
/* IRQ.C: IRQ Handlers                                                        */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2005-2007 Keil Software. All rights reserved.                */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/

#include <LH79524.H>                    /* NXP LH79524 register definitions   */

volatile unsigned char Clock;


__irq void TIMER0_IRQ_Handler (void) {  /* Timer0 interrupt handler           */

  static unsigned long ticks = 0;

  if (ticks++ >= (100-1)) {             /* Set Clock to 1 periodically        */
    ticks   = 0;
    Clock   = 1;
  }

  TIMER0->status  = 0x0004;             /* Clear Timer0 interrupt flag        */
  VIC->vectoraddr = 0;                  /* Acknowledge interrupt              */
}
