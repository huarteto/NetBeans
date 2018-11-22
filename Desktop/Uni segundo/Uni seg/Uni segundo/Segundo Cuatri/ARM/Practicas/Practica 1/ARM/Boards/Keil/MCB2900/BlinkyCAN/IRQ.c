/******************************************************************************/
/* IRQ.C: IRQ Handler                                                         */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2005-2007 Keil Software. All rights reserved.                */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/

#include <LPC29xx.H>


extern volatile unsigned char  Clock1s;
extern volatile          short AD_val;

extern volatile unsigned char  Rx_Data;

__irq void Timer0_Handler (void) {      /* Timer 0 Interrupt Handler          */
  static unsigned char cnt1s;

  if (cnt1s++ >= 99) {                  /* Set Clock1s each 1 second          */
    cnt1s   = 0;
    Clock1s = 1;
  }

  TIM0_INT_CLR_STATUS = 1;              /* Clear Timer 0 interrupt status     */
}


__irq void ADC2_Handler (void) {        /* ADC 2 Interrupt Handler            */

  AD_val = ADC2_ACD0;                   /* Read Conversion Result             */

  ADC2_INT_CLR_STATUS = 1;              /* Clear ADC2 interrupt status        */
}


__irq void CAN0_Rx_Handler (void) {     /* CAN0 Rx Interrupt Handler          */
  volatile unsigned long dummy;

  dummy = CAN0ICR;                      /* Read and clear interrupt status    */
  Rx_Data = CAN0RDA;                    /* Read Data Byte                     */
  CAN0CMR = 4;                          /* Release receive buffer             */
}
