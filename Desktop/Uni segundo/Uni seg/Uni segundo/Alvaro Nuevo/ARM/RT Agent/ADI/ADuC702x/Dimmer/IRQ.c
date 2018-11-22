/******************************************************************************/
/* IRQ.C: IRQ Handler for Dimmer Example                                      */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2005-2006 Keil Software. All rights reserved.                */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/

#include <ADuC7026.H>                 // ADuC7026 definitions
#include "RT_Agent.h"                 // Real-Time Agent definitions


volatile int T0_Tick;                 // Timer 0 Current Tick Value

__irq void IRQ_Handler (void) {
  if (IRQSIG & 0x00000004) {          // Timer 0 Interrupt
    T0CLRI = 1;                       // Clear Timer 0 Interrupt
    T0_Tick++;                        // Increment Timer 0 Tick
    
    RTA_rx_word_ext ();               // Call RTA polling functions
    RTA_tx_word_ext ();
  }


}

void RTA_irq_enable_ext (void) {
  IRQEN = 0x00000004;                 // Enable Timer 0 Interrupt    
}

void RTA_irq_disable_ext (void) {
  IRQCLR = 0x00000004;                // Disable Timer 0 Interrupt
}
