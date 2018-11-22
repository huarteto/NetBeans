/******************************************************************************/
/* SERIAL.C: Low Level Serial Routines                                        */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2005-2006 Keil Software. All rights reserved.                */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/

#include <S3C44B0X.H>                         /* S3C44B0X definitions */

#define MCLK 60000000                         /* Master Clock */
#define BR     115200                         /* Baud Rate */

#define BRD ((int)(MCLK/16.0/BR+0.5)-1)       /* Baud Rate Divisor */


regUART * pUART = pUART0;                     /* Global Pointer to UART0 */


void init_serial (void) {                     /* Initialize Serial Interface */

  pUART->ULCON  = NORMAL_MODE |               /* Normal Mode Operation */
                  PARITY_NONE |               /* No Parity   */
                  STOP_ONE    |               /* 1 Stop Bit  */
                  DATA_8;                     /* 8 Data Bits */
  pUART->UCON   = RXM_INTPOL  |               /* Rx Interrupt or Polling Mode */
                  TXM_INTPOL;                 /* Tx Interrupt or Polling Mode */
  pUART->UFCON  = FIFO_EN     |               /* FIFO Enable   */
                  TXF_RST     |               /* Tx FIFO Reset */
                  RXF_RST;                    /* Rx FIFO Reset */
  pUART->UBRDIV = BRD;                        /* Baud Rate Divisor */
}


int sendchar (int ch)  {                      /* Write character to
                                                  Serial Port */
  if (ch == '\n')  {                          /* Check for CR */
    while (!(pUART->UTRSTAT & TX_BUF_EMPTY)); /* Wait for Empty Tx Buffer */
    pUART->UTXH = '\r';                       /* Output CR */
  }
  while (!(pUART->UTRSTAT & TX_BUF_EMPTY));   /* Wait for Empty Tx Buffer */
  return (pUART->UTXH = ch);                  /* Transmit Character */
}


int getkey (void) {                           /* Read Character from
                                                  Serial Port */
  while (!(pUART->UTRSTAT & RX_BUF_READY));   /* Wait for Full Rx Buffer */
  return (pUART->URXH);                       /* Read Character */
}
