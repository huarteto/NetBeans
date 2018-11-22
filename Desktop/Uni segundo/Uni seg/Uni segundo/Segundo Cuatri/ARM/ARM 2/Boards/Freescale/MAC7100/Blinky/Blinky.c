/******************************************************************************/
/* BLINKY.C: LED Flasher                                                      */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2005-2006 Keil Software. All rights reserved.                */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/

#include <MAC7100.h>                    /* MAC7100 definitions                */
#include "IRQ.h"                        /* IRQ function prototypes            */


void PIT3_Handler (void) {              /* PIT3 timer interrupt routine       */
                                        /* gets called from common interrupt  */
  static unsigned short n = 0xFEFF;

  if (n == 0xFF00)  n = 0xFEFF;         /* LED Last On -> First On            */
  PIM_PF_PORTDATA = n;                  /* Turn on LED                        */
  n <<= 1;                              /* LED states shift                   */

  PIT_FLG |= 0x80;                      /* Clear PIT3 interrupt flag          */
}

                 
unsigned int Get_AD (void)  {           /* Get AD value form AD0              */
  
  ATD0_FLAG = 0;                        /* Clear flags register               */

  /* Start conversion on channel 0, 10 bit                                    */
  ATD0_CW = ATD_CWCH(0) | ATD_CWNF(0) | ATD_CWSL(3) | ATD_CWCM(1);

  while (!(ATD0_FLAG & 0x02));          /* Wait for conversion to end         */

  return (ATD0_RR & 0x3FF);             /* Return 10 bit AD result            */
}


int main (void) {
  unsigned int AD_Val, AD_Val_Ex;
  unsigned short i;

  /* Setup LEDs pins                                                          */
  PIM_PF_CONFIG8  = 0x60;               /* Setup pin as Output, Open drain    */
  PIM_PF_CONFIG9  = 0x60;               /* Setup pin as Output, Open drain    */
  PIM_PF_CONFIG10 = 0x60;               /* Setup pin as Output, Open drain    */
  PIM_PF_CONFIG11 = 0x60;               /* Setup pin as Output, Open drain    */
  PIM_PF_CONFIG12 = 0x60;               /* Setup pin as Output, Open drain    */
  PIM_PF_CONFIG13 = 0x60;               /* Setup pin as Output, Open drain    */
  PIM_PF_CONFIG14 = 0x60;               /* Setup pin as Output, Open drain    */
  PIM_PF_CONFIG15 = 0x60;               /* Setup pin as Output, Open drain    */
  PIM_PF_PORTDATA = 0xFFFF;             /* Turn all of the LEDs off           */

  /* Setup AD converter 0 (potentiometer)                                     */
  PIM_PE_CONFIG0 = 0x0080;              /* Set PORTE[0] to peripheral mode    */
  ATD0_PRE     = ATD_PRES(8);           /* AD 0, channel 0, prescale 4        */
  ATD0_MODE    = 0x00;                  /* AD enable                          */

  /* Setup timer 3 interrupt                                                  */
  PIT_CTRL    &= 0xFEFFFFFF;            /* Enable clock for timers            */
  PIT_INTSEL  |= 0x08;                  /* Enable PIT3 to generate interrupt  */  
  PIT_INTEN   |= 0x08;                  /* Enable PIT3 Timer                  */

  Activate_IRQ (21, 1, PIT3_Handler);   /* Setup PIT3 interrupt               */

  INTC_ICONFIG = INTC_EMASK | INTC_FIQDEF(0x1F);  /* Define operating config  */

  PIT_TLVAL3   = 0xFFF;                 /* PIT3 timer initial reload          */
  PIT_EN      |= 0x08;                  /* Enable PIT3 timer                  */

  for (i = 0; i < 275; i++);            /* Initial delay for AD to become     */
                                        /* ready, min 20 us (275 at 40 MHz)   */

  while (1) {                           /* Loop forever                       */
    AD_Val = Get_AD();                  /* Get AD conversion result           */

    if (AD_Val != AD_Val_Ex)  {         /* If AD value changed                */
      PIT_TLVAL3 = 0x8000+(AD_Val<<10); /* Calculate timer value upon AD value*/
                                        /* and correct timer reload register  */

      AD_Val_Ex = AD_Val;
    }
  }
}
