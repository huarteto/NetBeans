/******************************************************************************/
/* BLINKY.C: 7-segment LED Display Flasher                                    */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2005-2006 Keil Software. All rights reserved.                */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/

#include <ML674061.H>                       /* ML67Q405x/406x definitions */
#include "Blinky.h"
#include "IRQ.h"

#define CCLK    32000000                    /* CPU_CLK [Hz] */
#define CYCLE        500                    /* Count Cycle Time [ms] */
#define TMRVAL  (0x10000 - (CCLK/16/1000))  /* System Timer Reload Value */


/* 7-segment LED Display Pattern Table */
const unsigned char LED_Table[16] = {
  LED_0, LED_1, LED_2, LED_3, LED_4, LED_5, LED_6, LED_7,
  LED_8, LED_9, LED_A, LED_b, LED_C, LED_d, LED_E, LED_F
};

volatile unsigned long Tick1ms;             /* 1ms Time Tick */
volatile unsigned long ExtIntTick;          /* External Interrupt Tick */
volatile unsigned long Counter;             /* Counter Value */
volatile unsigned long Direction;           /* Count Direction */
                                            /*  0 - Up, 1 - Down */


/*
 * System Timer Interrupt Service Routine
 *   executed every 1ms
 */

void systimer_isr (void) {

  if (REG(TMOVF) & TMOVF_OVF) {             /* Check for Timer Overflow */
    Tick1ms++;                              /* Increment 1ms Tick */
    REG(TMOVF) = TMOVF_OVF;                 /* Clear Timer Overflow */
  }
}


/*
 * Setup System Timer
 */

void systimer_init (void) {

  IRQ_ISR[0] = &systimer_isr;               /* nIRQ[0]: ISR */
  REG(ILC0) |= ILC0_ILR0 & ILC0_INT_LV1;    /* nIRQ[0]: Level 1 */
  REG(TMRLR) = TMRVAL;                      /* Set Reload Value */
  REG(TMEN)  = TMEN_TCEN;                   /* Start Timer */
}


/*
 * External Interrupt 1 Service Routine
 *   executed on EXINT1 Falling Edge
 */

void extint1_isr (void) {

  ExtIntTick++;                             /* Increment Ext. Interrupt Tick */
  REG(EXIRQB) = IRQB_IRQ34;                 /* nIRQ[34]: Clear Interrupt */
}


/*
 * Setup External Interrupt 1
 */

void exint1_init (void) {

  IRQ_ISR[34]  = &extint1_isr;              /* nIRQ[34]: ISR */
  REG(EXIDMA) |= IDM_IDM34;                 /* nIRQ[34]: Falling Edge */
  REG(EXIRQB)  = IRQB_IRQ34;                /* nIRQ[34]: Clear Interrupt */
  REG(EXILCB) |= ILC_ILC34 & ILC_INT_LV1;   /* nIRQ[34]: Level 1 */
}


/*
 * Wait Function
 *   Parameter: Wait Time in ms
 */

void wait (unsigned long time) {
  unsigned long tick;

  tick = Tick1ms;                           /* Initial 1ms Tick */
  while ((Tick1ms - tick) < time);          /* Wait for specified Time */
}


/*
 * LED Initialization
 */

void LED_Init (void) {

  REG(PM4) |= LED_M4;
  REG(PM5) |= LED_M5;
  REG(PM3) |= LED_M3;
}


/*
 * LED Display
 *   Parameter: LED Num
 */

void LED_Display (unsigned long num) {
  unsigned long val;

  val  = LED_Table[num & 0x0F];
  REG(PO4) = (REG(PO4) & ~LED_M4) | ((num >> 0) & LED_M4);
  REG(PO5) = (REG(PO5) & ~LED_M5) | ((val >> 0) & LED_M5);
  REG(PO3) = (REG(PO3) & ~LED_M3) | ((val >> 3) & LED_M3);
}


/*
 * Main Program
 */

int main (void) {

  systimer_init();                          /* System Timer Initialization */
  exint1_init();                            /* Ext. Int. 1 Initialization */
  LED_Init();                               /* LED Initialization */

  for (;;) {                                /* Loop forever */
    LED_Display(Counter);                   /* Display Counter */
    wait(CYCLE);                            /* Wait */
    if (ExtIntTick) {                       /* Check for at least 1 Ext. Int. */
      ExtIntTick = 0;                       /* Clear External Interrupt Tick */
      Direction ^= 1;                       /* Change Direction */
    }
    if (Direction == 0) {                   /* Check Direction */
      Counter++;                            /* Count Up */
    } else {
      Counter--;                            /* Count Down */
    }
  }
}
