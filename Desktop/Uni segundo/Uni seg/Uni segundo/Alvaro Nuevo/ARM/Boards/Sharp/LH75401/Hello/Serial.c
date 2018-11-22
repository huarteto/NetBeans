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

#define CPU_CLK 51609600                 /* CPU Clock */
#define BAUD_R    115200                 /* Baud Rate */

#define BRGDIV  (CPU_CLK/BAUD_R/16)      /* Baud Rate Generator Divisor */

#define CR      0x0D                     /* Carriage Return ASCI Code */


void init_serial (void)  {               /* Initialize Serial Interface   */
  UART2_GIR  = 0x61;                     /* Select Register Bank 3        */
  UART2_CLCF = 0x50;                     /* Use BRGA for Rx & Tx Clock    */
  UART2_BACF = 0x04;                     /* BRGA in Baud Rate Gen. Mode   */
  UART2_GIR  = 0x01;                     /* Select Register Bank 0        */
  UART2_LCR  = 0x83;                     /* 8 bits, no Parity, 1 Stop bit */
  UART2_BAL  = BRGDIV & 0xFF;            /* Set Baud Rate A Divisor, LSB  */
  UART2_BAH  = BRGDIV >> 8;              /* Set Baud Rate A Divisor, MSB  */
  UART2_LCR  = 0x03;                     /* DLAB = 0                      */
}


int sendchar (int ch)  {                 /* Write character to Serial Port */

  if (ch == '\n')  {                     /* Check for CR */
    while (!(UART2_LSR & 0x20));         /* Wait for Empty Transmitter */
    UART2_TXD = CR;                      /* Output CR */
  }
  while (!(UART2_LSR & 0x20));           /* Wait for Empty Transmitter */
  return (UART2_TXD = ch);               /* Transmit Character */
}


int getkey (void)  {                     /* Read Character from Serial Port */

  while (!(UART2_LSR & 0x01));           /* Wait for Character */

  return (UART2_RXD);                    /* Read Character */
}
