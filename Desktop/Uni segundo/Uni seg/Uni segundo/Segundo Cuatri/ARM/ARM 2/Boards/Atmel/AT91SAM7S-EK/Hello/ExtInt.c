/******************************************************************************/
/* EXTINT.C: External Interrupt                                               */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2005-2006 Keil Software. All rights reserved.                */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/

#include <AT91SAM7S64.H>                    /* AT91SAM7S64 definitions */
#include "..\Board.h"


extern AT91S_PIO * pPIO;                    /* Global Pointer to PIO */

extern __irq void irq0_int (void);          /* IRQ0 Function */


void irq0_nint (void) {                     /* Nested IRQ0 (Push button SW2) */
  
  if ((pPIO->PIO_PDSR & SW2) == 0) {        /* Check if SW2 is pressed   */
    pPIO->PIO_CODR = LED2;                  /* Turn On LED2              */
    while ((pPIO->PIO_PDSR & SW2) == 0);    /* Wait until SW2 is pressed */
    pPIO->PIO_SODR = LED2;                  /* Turn Off LED2             */
  }

  *AT91C_AIC_EOICR = 0;                     /* End of Interrupt          */
}


void init_extint (void) {                   /* Setup IRQ 0 Interrupt */
  AT91S_AIC * pAIC = AT91C_BASE_AIC;

  /* Setup IRQ0 Interrupt Mode and Vector with Priority 0 and Enable it */
  pAIC->AIC_SMR[AT91C_ID_IRQ0] = AT91C_AIC_SRCTYPE_INT_POSITIVE_EDGE | 0;
  pAIC->AIC_SVR[AT91C_ID_IRQ0] = (unsigned long) irq0_int;
  pAIC->AIC_IECR = (1 << AT91C_ID_IRQ0);
}
