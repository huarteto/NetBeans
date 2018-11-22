/******************************************************************************/
/* SERIAL.C: Low Level Serial Routines                                        */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2005-2006 Keil Software. All rights reserved.                */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/

#include <91M55800.H>                      /* AT91M55800A definitions         */

#define CR     0x0D


void init_serial (void)  {                 /* Initialize Serial Interface     */
  PIOA_PDR = (1<<PIORXD0) | (1<<PIOTXD0);  /* Enable RXD0 and TXD0            */
  US0_MR   = US_CHRL_8 | US_PAR_NO;        /* 8 bits, no Parity, 1 Stop bit   */
  US0_BRGR = 208;                          /* 9600 Baud Rate @ 20 MHz Clock   */
  US0_CR   = US_RXEN | US_TXEN;            /* Enable RX and TX                */
}


/* implementation of putchar (also used by printf function to output data)    */
int sendchar (int ch)  {                   /* Write character to Serial Port  */

  if (ch == '\n')  {
    while (!(US0_CSR & US_TXRDY));
    US0_THR = CR;                          /* output CR */
  }
  while (!(US0_CSR & US_TXRDY));
  return (US0_THR = ch);
}


int getkey (void)  {                       /* Read character from Serial Port */

  while (!(US0_CSR & US_RXRDY));
  return (US0_RHR);
}
