/******************************************************************************/
/* IRQ.C: Common IRQ Handler                                                  */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2005-2007 Keil Software. All rights reserved.                */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/

#include <S3C2440.H>


volatile unsigned long ticks = 0;
volatile          char Speed = 12;
volatile unsigned char Clock;


/* Common interrupt handler, handles all interrupts                           */
__irq void IRQ_Handler (void) {

  unsigned int msk = (1 << INTOFFSET);

  switch (msk) {
    case BIT_TIMER0:                    /* Timer 0 interrupt mask             */
      if (ticks++ >= ((Speed << 3)+4)) {/* Set Clock to 1 periodically        */
        ticks   = 0;
        Clock   = 1;
      }
      break;
  }

  SRCPND = msk;                         /* Clear Interrupt                   */
  INTPND = msk;
}
