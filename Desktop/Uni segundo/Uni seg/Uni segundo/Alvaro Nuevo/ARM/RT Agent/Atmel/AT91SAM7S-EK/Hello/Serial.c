/******************************************************************************/
/* SERIAL.C: Low Level Serial Routines                                        */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2005-2006 Keil Software. All rights reserved.                */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/

#include <AT91SAM7S64.H>                    /* AT91SAM7S64 definitions */
#include "..\Board.h"

#define BR    115200                        /* Baud Rate */

#define BRD  (MCK/16/BR)                    /* Baud Rate Divisor */


AT91S_USART * pUSART = AT91C_BASE_US0;      /* Global Pointer to USART0 */


void init_serial (void) {                   /* Initialize Serial Interface */

  *AT91C_PIOA_PDR = AT91C_PA5_RXD0 |        /* Enable RxD0 Pin */
                    AT91C_PA6_TXD0;         /* Enalbe TxD0 Pin */

  pUSART->US_CR = AT91C_US_RSTRX |          /* Reset Receiver      */
                  AT91C_US_RSTTX |          /* Reset Transmitter   */
                  AT91C_US_RXDIS |          /* Receiver Disable    */
                  AT91C_US_TXDIS;           /* Transmitter Disable */

  pUSART->US_MR = AT91C_US_USMODE_NORMAL |  /* Normal Mode */
                  AT91C_US_CLKS_CLOCK    |  /* Clock = MCK */
                  AT91C_US_CHRL_8_BITS   |  /* 8-bit Data  */
                  AT91C_US_PAR_NONE      |  /* No Parity   */
                  AT91C_US_NBSTOP_1_BIT;    /* 1 Stop Bit  */

  pUSART->US_BRGR = BRD;                    /* Baud Rate Divisor */

  pUSART->US_CR = AT91C_US_RXEN  |          /* Receiver Enable     */
                  AT91C_US_TXEN;            /* Transmitter Enable  */
}


int sendchar (int ch)  {                    /* Write character to Serial Port */

  if (ch == '\n')  {                            /* Check for CR */
    while (!(pUSART->US_CSR & AT91C_US_TXRDY)); /* Wait for Empty Tx Buffer */
    pUSART->US_THR = '\r';                      /* Output CR */
  }
  while (!(pUSART->US_CSR & AT91C_US_TXRDY));   /* Wait for Empty Tx Buffer */
  return (pUSART->US_THR = ch);                 /* Transmit Character */
}


int getkey (void)  {                        /* Read character from Serial Port */

  while (!(pUSART->US_CSR & AT91C_US_RXRDY));   /* Wait for Full Rx Buffer */
  return (pUSART->US_RHR);                      /* Read Character */
}
