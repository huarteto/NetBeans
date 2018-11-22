/******************************************************************************/
/* TIME.C: Time Functions for 1000Hz Clock Tick                                */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2005-2006 Keil Software. All rights reserved.                */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/

#include <AT91SAM7S64.H>                    /* AT91SAM7S64 definitions */
#include "..\Board.h"

#define TCK  1000                           /* Timer Clock  */

#define PIV  ((MCK/TCK/16)-1)               /* Periodic Interval Value */


volatile unsigned long timeval;             /* Current Time Tick */


__irq void system_int (void) {              /* System Interrupt Handler */
  AT91S_PITC * pPIT = AT91C_BASE_PITC;

  if (pPIT->PITC_PISR & AT91C_PITC_PITS) {  /* Check PIT Interrupt */
    timeval++;                              /* Increment Time Tick */
    if ((timeval % 500) == 0) {             /* 500ms Elapsed ? */
      *AT91C_PIOA_ODSR ^= LED4;             /* Toggle LED4 */
    }
    *AT91C_AIC_EOICR = pPIT->PITC_PIVR;     /* Ack & End of Interrupt */
  } else {
    *AT91C_AIC_EOICR = 0;                   /* End of Interrupt */
  }
}


void init_timer (void) {                    /* Setup PIT with Interrupt */
  AT91S_AIC * pAIC = AT91C_BASE_AIC;

  *AT91C_PITC_PIMR = AT91C_PITC_PITIEN |    /* PIT Interrupt Enable */
                     AT91C_PITC_PITEN  |    /* PIT Enable */
                     PIV;                   /* Periodic Interval Value */

  /* Setup System Interrupt Mode and Vector with Priority 7 and Enable it */
  pAIC->AIC_SMR[AT91C_ID_SYS] = AT91C_AIC_SRCTYPE_INT_POSITIVE_EDGE | 7;
  pAIC->AIC_SVR[AT91C_ID_SYS] = (unsigned long) system_int;
  pAIC->AIC_IECR = (1 << AT91C_ID_SYS);
}
