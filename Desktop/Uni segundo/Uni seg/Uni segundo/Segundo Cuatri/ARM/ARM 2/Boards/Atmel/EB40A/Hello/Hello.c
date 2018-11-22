/******************************************************************************/
/* HELLO.C: Hello World Example                                               */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2005-2006 Keil Software. All rights reserved.                */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/

#include <stdio.h>                         /* I/O Functions */
#include <91R40008.H>                      /* AT91R40008 Definitions */


void delay (void)  {                       /* Delay Function */
  unsigned long x;

  for (x = 0; x < 400000; x++);
}


int main (void)  {                         /* Main Program */

  /* Initialize Serial Interface */
  PIO_PDR  = (1<<PIORXD0) | (1<<PIOTXD0);  /* Enable RXD and TXD              */
  US0_MR   = US_CHRL_8 | US_PAR_NO;        /* 8 bits, no Parity, 1 Stop bit   */
  US0_BRGR = 430;                          /* 9600 Baud Rate @ 66 MHz Clock   */
  US0_CR   = US_RXEN | US_TXEN;            /* Enable RX and TX                */

  PIO_PER  = 0x00010000;                   /* Enable P16 */
  PIO_OER  = 0x00010000;                   /* Set P16 as Output */

  while (1) {
    PIO_CODR = 0x00010000;                 /* Turn LED D1 On  (P16 = 0) */
    printf ("Hello World\n");              /* Print "Hello World" */
    PIO_SODR = 0x00010000;                 /* Turn LED D1 Off (P16 = 1) */    
    delay();                               /* Delay */
  }
}
