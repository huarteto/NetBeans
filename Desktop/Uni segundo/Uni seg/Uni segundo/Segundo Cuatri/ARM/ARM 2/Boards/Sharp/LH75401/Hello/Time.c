/******************************************************************************/
/* TIME.C: Time Functions for 100Hz Clock Tick                                */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2005-2006 Keil Software. All rights reserved.                */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/

#include <LH75401.H>                        // LH75401 definitons

#define CPU_CLK 51609600                    // CPU Clock
#define TIM_CLK 100                         // Timer Clock

#define TIM_CNT (CPU_CLK/TIM_CLK/128)       // Timer Counts


volatile unsigned long timeval;             // Current Time Tick


/* Timer 0 Interrupt executes each 10ms */
 __irq void timer0 (void) {
  timeval++;                                // Increment Time Tick
  TM0_STATUS  = 0x0004;                     // Clear Interrupt flag
  VICVectAddr = 0;                          // Acknowledge Interrupt
}


/* Setup the Timer 0 Interrupt */
void init_timer (void) {

  TM0_CMP_CAP_CNTRL = 0x4000;               // Clear TM0_CNT upon TM0_CMP1 match
  TM0_INT_CNTRL     = 0x0004;               // Interrupt for TM0_CMP1 match
  TM0_CMP1          = TIM_CNT - 1;          // Timer Counts for TIM_CLK
  TM0_CNTRL         = 0x001A;               // Start Timer 0, Prescaler = 128

  VICVectAddr0 = (unsigned long)timer0;     // Set Interrupt Vector in Slot 0
  VICVectCntl0 = 0x20 | TIMER0_INT;         // use it for Timer 0 Interrupt
  VICIntEnable = 1 << TIMER0_INT;           // Enable Timer 0 Interrupt
}
