/******************************************************************************/
/* TIME.C: Time Functions for 1000Hz Clock Tick                               */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2005-2006 Keil Software. All rights reserved.                */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/

#include <LPC21XX.H>                           // LPC21XX Peripheral Registers

long volatile timeval;

/* Timer Counter 0 Interrupt executes each 1ms @ 60 MHz CPU Clock */
__irq void tc0 (void) {
  timeval++;
  T0IR = 1;                                    // Clear interrupt flag
  VICVectAddr = 0;                             // Acknowledge Interrupt
}

/* Setup the Timer Counter 0 Interrupt */
void init_timer (void) {
  T0MR0 = 14999;                               // 1mSec = 15.000-1 counts
  T0MCR = 3;                                   // Interrupt and Reset on MR0
  T0TCR = 1;                                   // Timer0 Enable
  VICVectAddr0 = (unsigned long)tc0;           // set interrupt vector in 0
  VICVectCntl0 = 0x20 | 4;                     // use it for Timer 0 Interrupt
  VICIntEnable = 0x00000010;                   // Enable Timer0 Interrupt
}
