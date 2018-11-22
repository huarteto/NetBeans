/******************************************************************************/
/* IRQ.C: IRQ Handler                                                         */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2005-2006 Keil Software. All rights reserved.                */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/

#include <MAC7100.h>                    /* MAC7100 header file                */


static void (*IRQ_Table[64]) (void);    /* Interrupt vector table             */


__irq void IRQ_Handler (void) {         /* Common interrupt routine           */

  (IRQ_Table[INTC_IRQIACK-64]) ();      /* Call appropriate interrupt function*/
  INTC_CLMASK = INTC_SLMASK;            /* Clear Current Level Mask Register  */
}


/* Function that setups and activates interrupt                               */
void Activate_IRQ (int channel, int level, void (*IRQ_Function) ())  {

  IRQ_Table[channel] = IRQ_Function;    /* Make entry to IRQ_Table            */
  INTC_ICR(channel)  = level;           /* Setup interrupt level              */
  INTC_CIMR          = channel;         /* Enable interrupt                   */
}


/* Function that dectivates interrupt                                         */
void Deactivate_IRQ (int channel)  {

  INTC_SIMR          = channel;         /* Disable interrupt                  */
}

