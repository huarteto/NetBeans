/******************************************************************************/
/* IRQ.C: IRQ Handler for Dimmer Example                                      */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2005-2006 Keil Software. All rights reserved.                */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/

#include <ADuC7026.H>               // ADuC7026 definitions


volatile int T0_Tick;               // Timer 0 Current Tick Value

__irq void IRQ_Handler (void) {
  if (IRQSIG & 0x00000004) {        // Timer 0 Interrupt
    T0CLRI = 1;                     // Clear Timer 0 Interrupt
    T0_Tick++;                      // Increment Timer 0 Tick
  }
}
