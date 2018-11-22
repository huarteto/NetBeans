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
#include "..\Board.h"                       /* Development board definititons */
#include "RT_Agent.h"                       /* Real-Time Agent definitions */

#define TCK  1000                           /* Timer Clock  */

#define PIV  ((MCK/TCK/16)-1)               /* Periodic Interval Value */


volatile unsigned long timeval;             /* Current Time Tick */


__irq void system_int (void) {              /* System Interrupt Handler */
  volatile AT91S_PITC * pPIT = AT91C_BASE_PITC;
  
  
  // Process COMMRX interrupt if any (Real-Time Agent) 
  if (*AT91C_DBGU_CSR & *AT91C_DBGU_IMR & AT91C_US_COMM_RX) {
    RTA_rx_word_ext();
  }

  // Process COMMTX interrupt if any (Real-Time Agent)
  if (*AT91C_DBGU_CSR & *AT91C_DBGU_IMR & AT91C_US_COMM_TX) {
    RTA_tx_word_ext();
  }
  
  // Process PIT interrupt if any
  if (pPIT->PITC_PISR & AT91C_PITC_PITS) {  /* Check PIT Interrupt */
    timeval++;                              /* Increment Time Tick */
    if ((timeval % 500) == 0) {             /* 500ms Elapsed ? */
      *AT91C_PIOA_ODSR ^= LED4;             /* Toggle LED4 */
    }
    pPIT->PITC_PIVR;                        /* Ack PIT Interrupt */
  }

  *AT91C_AIC_EOICR = 0;  
}

void init_system_int (void) {               /* Setup System Interrupt */
  AT91S_AIC * pAIC = AT91C_BASE_AIC;

  /* Setup System Interrupt Mode and Vector with Priority 7 highest and 
     enable it ...
     NOTE: Interrupt must be level sensitive as we have or'd sources. If
     it was edge triggered, one of the edges could be missed, and therefore
     the interrupt missed. Level triggering means the handler will be 
     re-executed until all sources have been processed */
  pAIC->AIC_SMR[AT91C_ID_SYS] = AT91C_AIC_SRCTYPE_INT_HIGH_LEVEL | AT91C_AIC_PRIOR_HIGHEST;
  pAIC->AIC_SVR[AT91C_ID_SYS] = (unsigned long) system_int;
  pAIC->AIC_IECR = (1 << AT91C_ID_SYS);


  *AT91C_PITC_PIMR = AT91C_PITC_PITIEN |    /* PIT Interrupt Enable */
                     AT91C_PITC_PITEN  |    /* PIT Enable */
                     PIV;                   /* Periodic Interval Value */


}
