/******************************************************************************/
/* TIME.C: Time Functions for 1000Hz Clock Tick                               */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2005-2006 Keil Software. All rights reserved.                */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/

#include <S3C44B0X.H>                       /* S3C44B0X definitions */

#define MCLK 60000000                       /* Master Clock */
#define TCLK     1000                       /* Timer Clock  */

#define TCNT (MCLK/TCLK/2)                  /* Timer Counts */


volatile unsigned long timeval;             /* Current Time Tick */


__irq void HandlerTIMER5 (void) {           /* Timer 5 Interrupt Handler */

  timeval++;                                /* Increment Time Tick  */
  pIC->I_ISPC = INT_TIMER5;                 /* Clear Interrupt Flag */
}


void init_timer (void) {                    /* Setup Timer 5 with Interrupt */

  pPWM->TCNTB5 = TCNT - 1;                  /* Timer 5 Period */
  pPWM->TCON   = T5_UPDATE;                 /* Update TCNCTB5 */
  pPWM->TCON   = T5_START | T5_AUTOREL;     /* Start Timer 5 (Auto Reload) */

  pIC->INTMSK &= ~(INT_GLOBAL |             /* Global Interrupt Enable  */
                   INT_TIMER5);             /* Timer 5 Interrupt Enable */
  pIC->INTCON  = FIQ_DISABLE |              /* Disable FIQ */
                 IRQ_ENABLE  |              /* Enable IRQ  */
                 VECT_IRQ;                  /* IRQ Vectored Interrupt Mode */
}
