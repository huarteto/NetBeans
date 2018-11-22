/******************************************************************************/
/* SERIAL.C: Low Level Serial Routines                                        */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2005-2006 Keil Software. All rights reserved.                */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/

#include <91R40008.H>                      /* AT91R40008 definitions          */

#define CR     0x0D


/* implementation of putchar (also used by printf function to output data)    */
int sendchar (int ch)  {                 /* Write character to Serial Port    */

  if (ch == '\n')  {
    while (!(US0_CSR & US_TXRDY));
    US0_THR = CR;                        /* output CR */
  }
  while (!(US0_CSR & US_TXRDY));
  return (US0_THR = ch);
}


int getkey (void)  {                     /* Read character from Serial Port   */

  while (!(US0_CSR & US_RXRDY));
  return (US0_RHR);
}
