/******************************************************************************/
/* Timer.c: Low Level Timer Routines                                          */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2009-2010 KEIL - An ARM Company. All rights reserved.        */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/

#include "NUC501.h"                          /* NUC501 settings               */
#include "IRQ.h"

void TMR0_IRQ_Handler (void);
void TMR1_IRQ_Handler (void);

#define CCLK    12000000                     /* XTAL [Hz]                     */
#define TMRVAL_1MS  ((CCLK/1000))            /* System Timer Reload Value 1ms */

volatile uint16_t  clock_1s;                 /* Flag activated each second    */

/*----------------------------------------------------------------------------
  Initialize 
 *----------------------------------------------------------------------------*/
void TIM_init(int chn, int ms) {

  NUC_CLK->APBCLK |= (1UL << 0);             /* enable clock for Timer        */ 

  if (chn == 0) {
    NUC_TMR->TCSR0 = (1UL << 29) |           /* enable interrupt              */
                     (1UL << 27) |           /* set periodic mode             */ 
                     (0UL <<  0);            /* set prescaler to 1            */
    NUC_TMR->TICR0 = (TMRVAL_1MS * ms);

    NUC_AIC->MECR  |= (1UL << NUC_INT_TMR0);   /* enable Timer 0 IRQ          */
    IRQ_ISR[NUC_INT_TMR0] = &TMR0_IRQ_Handler; /* nIRQ[15]: Timer 0 ISR       */
  }
  else {
    NUC_TMR->TCSR1 = (1UL << 29) |           /* enable interrupt              */
                     (1UL << 27) |           /* set periodic mode             */ 
                     (0UL <<  0);            /* set prescaler to 1            */
    NUC_TMR->TICR1 = (TMRVAL_1MS * ms);

    NUC_AIC->MECR  |= (1UL << NUC_INT_TMR1);   /* enable Timer 1 IRQ          */
    IRQ_ISR[NUC_INT_TMR1] = &TMR1_IRQ_Handler; /* nIRQ[14]: Timer 1 ISR       */
  }

}

/*----------------------------------------------------------------------------
  start timer 
 *----------------------------------------------------------------------------*/
void TIM_start(int chn) {

  if (chn == 0) {
    NUC_TMR->TISR   = (1UL << chn);          /* clear timer interrupt         */
    NUC_TMR->TCSR0 |= (1UL << 26);           /* reset timer                   */
    NUC_TMR->TCSR0 |= (1UL << 30);           /* start timer                   */
  }
  else {
    NUC_TMR->TISR   = (1UL << chn);          /* clear timer interrupt         */
    NUC_TMR->TCSR1 |= (1UL << 26);           /* reset timer                   */
    NUC_TMR->TCSR1 |= (1UL << 30);           /* start timer                   */
  } 
}

/*----------------------------------------------------------------------------
  Timer0 IRQ Handler 
 *----------------------------------------------------------------------------*/
void TMR0_IRQ_Handler (void) {
  static   uint32_t ticks;

  NUC_TMR->TISR = (1UL << 0);                /* clear timer interrupt         */

  if (ticks++ >= 99) {                       /* check for 1 second            */
    ticks    = 0;
    clock_1s = 1;
  }

}

/*----------------------------------------------------------------------------
  Timer1 IRQ Handler 
 *----------------------------------------------------------------------------*/
void TMR1_IRQ_Handler (void) {
  static   uint32_t ticks;

  NUC_TMR->TISR = (1UL << 1);                /* clear timer interrupt         */

  if (ticks++ >= 99) {                       /* check for 1 second            */
    ticks    = 0;
    clock_1s = 1;
  }

}
