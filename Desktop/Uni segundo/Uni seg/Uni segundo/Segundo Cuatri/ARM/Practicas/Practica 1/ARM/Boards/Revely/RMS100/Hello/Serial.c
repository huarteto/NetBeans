/******************************************************************************/
/* SERIAL.C: Low Level Serial Routines                                        */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2005-2006 Keil Software. All rights reserved.                */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/

#include <LH75401.H>                     /* LH75401 definitons */

#define UART_CLK  14765600               /* UART Clock */
#define BAUD_R    115200                 /* Baud Rate */

#define BRGDIV  (UART_CLK/BAUD_R/16)     /* Baud Rate Generator Divisor */

#define CR      0x0D                     /* Carriage Return ASCI Code */


void init_serial (void)  {               /* Initialize Serial Interface   */
  UART1->ibrd  = BRGDIV;                 /* Set Integer Baud Rate Divisor */
  UART1->lcr_h = 0x60;                   /* 8 bits, no Parity, 1 Stop bit */
  UART1->cr    = 0x0301;                 /* Enable UART: Rx & Tx */
}


int sendchar (int ch)  {                 /* Write character to Serial Port */

  if (ch == '\n')  {                     /* Check for CR */
    while (!(UART1->fr & 0x80));         /* Wait for Empty Transmitter */
    UART1->dr = CR;                      /* Output CR */
  }
  while (!(UART1->fr & 0x80));           /* Wait for Empty Transmitter */
  return (UART1->dr = ch);               /* Transmit Character */
}


int getkey (void)  {                     /* Read Character from Serial Port */

  while (!(UART1->fr & 0x40));           /* Wait for Character */

  return (UART1->dr & 0xFF);             /* Read Character */
}
