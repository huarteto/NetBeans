/******************************************************************************/
/* IRQ.C: IRQ Handler for SJA2510 BlinkyIRQ Example                           */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2005-2007 Keil Software. All rights reserved.                */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/

#include <SJA2510.H>


volatile unsigned long ticks = 0;
volatile          char Speed;
volatile unsigned char Clock;


/* Common interrupt handler, handles all interrupts                           */
__irq void IRQ_Handler (void) {

  switch ((INT_VECTOR_1 >> 3) & 0x0F) {
    case 2:                             // Timer 0 interrupt index
      if (ticks++ >= ((Speed << 3)+1)) {// Set Clock to 1 periodically
        ticks   = 0;
        Clock   = 1;
      }
      TIM0_INT_CLR_STATUS = 1;          // Clear Timer 0 interrupt status
      break;
  }
}
