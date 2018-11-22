/******************************************************************************/
/* TIME.C: Time Functions for 1000Hz Clock Tick                                */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2005-2006 Keil Software. All rights reserved.                */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/

#include <AT91SAM7X256.H>                        /* AT91SAMT7X256 definitions */
#include "..\AT91SAM7X-EK.h"

#define TCK  1000                           /* Timer Clock  */

#define PIV  ((AT91B_MCK/TCK/16)-1)         /* Periodic Interval Value */


volatile unsigned long timeval;             /* Current Time Tick */

extern AT91S_PIO * pPIO_Led;                /* Global Pointer to PIO */

__irq void pit_int (void) {                 /* System Interrupt Handler */
  AT91S_PITC * pPIT = AT91C_BASE_PITC;

  if (pPIT->PITC_PISR & AT91C_PITC_PITS) {  /* Check PIT Interrupt */
    timeval++;                              /* Increment Time Tick */
    if ((timeval % 500) == 0) {             /* 500ms Elapsed ? */
      pPIO_Led->PIO_ODSR ^= AT91B_LED4;     /* Toggle LED4 */
    }
    *AT91C_AIC_EOICR = pPIT->PITC_PIVR;     /* Ack & End of Interrupt */
  } else {
    *AT91C_AIC_EOICR = 0;                   /* End of Interrupt */
  }
}


__irq void tc0_int (void) {
  AT91S_TC* pTC0 = AT91C_BASE_TC0;

  pPIO_Led->PIO_ODSR ^= AT91B_LED3;         /* Toggle LED3 */

  *AT91C_AIC_EOICR = pTC0->TC_SR;           /* End of Interrupt */
}


void init_pit (void) {                      /* Setup PIT with Interrupt */
  AT91S_AIC * pAIC = AT91C_BASE_AIC;

  *AT91C_PITC_PIMR = AT91C_PITC_PITIEN |    /* PIT Interrupt Enable */
                     AT91C_PITC_PITEN  |    /* PIT Enable */
                     PIV;                   /* Periodic Interval Value */

  /* Setup System Interrupt Mode and Vector with Priority 7 and Enable it */
  pAIC->AIC_SMR[AT91C_ID_SYS] = AT91C_AIC_SRCTYPE_INT_POSITIVE_EDGE | 7;
  pAIC->AIC_SVR[AT91C_ID_SYS] = (unsigned long) pit_int;
  pAIC->AIC_IECR = (1 << AT91C_ID_SYS);
}


void init_tc0 (void) {                      /* Setup TC0 with Interrupt */
  AT91S_TC*  pTC0 = AT91C_BASE_TC0;
  AT91S_AIC* pAIC = AT91C_BASE_AIC;

  *AT91C_PMC_PCER |= (1 << AT91C_ID_TC0);    /* Enable Clock for TC0 */

  pTC0->TC_CCR  = AT91C_TC_CLKEN | AT91C_TC_SWTRG;
  pTC0->TC_CMR  = AT91C_TC_CLKS_TIMER_DIV4_CLOCK | AT91C_TC_CPCTRG; /* prescaler 128 */ 
  pTC0->TC_RC   = ((((double)(AT91B_MCK/128)*TCK*100UL)/1E6)-1);	/* 100 ms        */
  pTC0->TC_IER  = AT91C_TC_CPCS;

  /* Setup System Interrupt Mode and Vector with Priority 7 and Enable it */
  pAIC->AIC_SMR[AT91C_ID_TC0] = AT91C_AIC_SRCTYPE_INT_POSITIVE_EDGE | 0;
  pAIC->AIC_SVR[AT91C_ID_TC0] = (unsigned long) tc0_int;
  pAIC->AIC_IECR = (1 << AT91C_ID_TC0);
}
