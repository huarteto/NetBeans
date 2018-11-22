/******************************************************************************/
/* IRQ.C: IRQ Handlers                                                        */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2005-2007 Keil Software. All rights reserved.                */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/

#include <S3F4A.H>                      /* Samsung S3F4A register definitions */


volatile          char Speed = 5;
volatile unsigned char Clock;


__irq void ST1_IRQ_Handler (void) {     /* Simple Timer 1 interrupt handler   */

  static unsigned long ticks = 0;

  if (ticks++ >= ((11-Speed) << 3)) {   /* Set Clock to 1 periodically        */
    ticks   = 0;
    Clock   = 1;
  }

  CSP_ST16_SET_CSR  (ST1, ST16_CHEND1); /* Clear ST1 channel1 end interrupt   */
  CSP_GIC_SET_EOICR (GIC, 0);           /* End Interrupt                      */
}
