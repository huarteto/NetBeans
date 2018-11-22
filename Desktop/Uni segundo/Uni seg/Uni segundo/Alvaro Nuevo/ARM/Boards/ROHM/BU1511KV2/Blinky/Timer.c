/******************************************************************************/
/* Timer.c: Low Level Timer Routines                                        */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2009-2010 KEIL - An ARM Company. All rights reserved.        */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/

#include "BU1511KV2.h"                    /* BU1511KV2 settings               */
#include "IRQ.h"

void TMR1_IRQ_Handler (void);
void TMR2_IRQ_Handler (void);

#define CCLK    40500000                  /* CCLK [Hz]                        */
#define TMRVAL_1MS  ((CCLK/1000))         /* System Timer Reload Value 1ms    */

volatile uint16_t  clock_1s;              /* Flag activated each second       */

/*----------------------------------------------------------------------------
  Initialize 
 *----------------------------------------------------------------------------*/
void TIM_init(int timer, int ms) {
  ROHM_TMR_TypeDef *pTmr;
           uint32_t tmr;
  volatile uint32_t eoi;
   
  if (timer == 1) {
    tmr = 0; 
    pTmr = (ROHM_TMR_TypeDef *)ROHM_TMR1; /* use Timer#1                      */
    IRQ_ISR[0] = &TMR1_IRQ_Handler;       /* nIRQ[0]: TIMER#1 ISR             */
  }
  else {
    tmr = 1; 
    pTmr = (ROHM_TMR_TypeDef *)ROHM_TMR2; /* use Timer#1                      */
    IRQ_ISR[1] = &TMR2_IRQ_Handler;       /* nIRQ[1]: TIMER#2 ISR             */
  }


  pTmr->LoadCount  = (TMRVAL_1MS * ms);
  pTmr->ControlReg = ((1UL << 1) |        /* user definition mode             */
                      (0UL << 2)  );      /* enable interrupt                 */

  eoi = pTmr->EOI;                        /* clear timer interrupt            */

  ROHM_INTCTL->irq_intmask_I &= ~(1UL << tmr);  /* unmask interrupt           */
  ROHM_INTCTL->irq_inten_I   |=  (1UL << tmr);  /* enable interrupt           */
}

/*----------------------------------------------------------------------------
  start timer 
 *----------------------------------------------------------------------------*/
void TIM_start(int timer) {
  ROHM_TMR_TypeDef *pTmr;
  volatile uint32_t eoi;

  pTmr = (timer == 1) ? (ROHM_TMR_TypeDef *)ROHM_TMR1 : (ROHM_TMR_TypeDef *)ROHM_TMR2;

  eoi = pTmr->EOI;                        /* clear timer interrupt            */
  pTmr->ControlReg |= (1UL << 0);         /* start timer                      */
}

/*----------------------------------------------------------------------------
  Timer1 IRQ Handler 
 *----------------------------------------------------------------------------*/
void TMR1_IRQ_Handler (void) {
  volatile uint32_t reg;
  static   uint32_t ticks;

  reg = ROHM_TMR1->STAT;                  /* read IRQ status                  */
  reg = ROHM_TMR1->EOI;                   /* clear interrupt                  */

  if (ticks++ >= 99) {                    /* check for 1 second               */
    ticks    = 0;
    clock_1s = 1;
  }

}

/*----------------------------------------------------------------------------
  Timer2 IRQ Handler 
 *----------------------------------------------------------------------------*/
void TMR2_IRQ_Handler (void) {
  volatile uint32_t reg;
  static   uint32_t ticks;

  reg = ROHM_TMR2->STAT;                  /* read IRQ status                  */
  reg = ROHM_TMR2->EOI;                   /* clear interrupt                  */

  if (ticks++ >= 99) {                    /* check for 1 second               */
    ticks    = 0;
    clock_1s = 1;
  }

}
