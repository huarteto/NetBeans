/******************************************************************************/
/* BLINKY.C: 7-segment LED Display Flasher                                    */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2005-2006 Keil Software. All rights reserved.                */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/

#include <ML675001.H>                       /* ML67500X definitions */
#include "Blinky.h"
#include "IRQ.h"

#define CCLK    60000000                    /* CCLK [Hz] */
#define CYCLE        500                    /* Count Cycle Time [ms] */
#define TMRVAL  (0x10000 - (CCLK/16/1000))  /* System Timer Reload Value */


/* 7-segment LED Display Pattern Table */
const unsigned char LED_Table[16] = {
  LED_0, LED_1, LED_2, LED_3, LED_4, LED_5, LED_6, LED_7,
  LED_8, LED_9, LED_A, LED_b, LED_C, LED_d, LED_E, LED_F
};

volatile unsigned long Tick1ms;             /* 1ms Time Tick */
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
 * External Interrupt 0 Service Routine
 *   executed on EXINT0 Falling Edge
 */

void extint0_isr (void) {

  Direction ^= 1;                           /* Change Direction */
  REG(IRQA)  = IRQA_IRQ22;                  /* nIRQ[22]: Clear Interrupt */
}


/*
 * Setup External Interrupt 0
 */

void exint0_init (void) {

  IRQ_ISR[22] = &extint0_isr;               /* nIRQ[22]: ISR */
  REG(IDM)   |= IDM_IDM22;                  /* nIRQ[22]: Falling Edge */
  REG(IRQA)   = IRQA_IRQ22;                 /* nIRQ[22]: Clear Interrupt */
  REG(ILC)   |= ILC_ILC22 & ILC_INT_LV1;    /* nIRQ[22]: Level 1 */
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

  REG(GPPMB) = (REG(GPPMB) & ~LED_BM) | LED_BM;
  REG(GPPMC) = (REG(GPPMC) & ~LED_CM) | LED_CM;
}


/*
 * LED Display
 *   Parameter: LED Num
 */

void LED_Display (unsigned long num) {
  unsigned long val;

  val = LED_Table[num & 0x0F];
  REG(GPPOB) = (REG(GPPOB) & ~LED_BM) | ((val >> 0) & LED_BM);
  REG(GPPOC) = (REG(GPPOC) & ~LED_CM) | ((val >> 6) & LED_CM);
}


/*
 * Main Program
 */

int main (void) {

  systimer_init();                          /* System Timer Initialization */
  exint0_init();                            /* Ext. Int. 0 Initialization */
  LED_Init();                               /* LED Initialization */

  for (;;) {                                /* Loop forever */
    LED_Display(Counter);                   /* Display Counter */
    wait(CYCLE);                            /* Wait */
    if (Direction == 0) {                   /* Check Direction */
      Counter++;                            /* Count Up */
    } else {
      Counter--;                            /* Count Down */
    }
  }
}
