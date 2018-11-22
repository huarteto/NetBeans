/******************************************************************************/
/* Serial.c: Low Level Serial Routines                                        */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2009-2010 KEIL - An ARM Company. All rights reserved.        */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/

#include "NUC501.h"                          /* NUC501 settings               */


/*----------------------------------------------------------------------------
  Initialize UART pins, Baudrate
 *----------------------------------------------------------------------------*/
void SER_init (int uart) {
  NUC_UART_TypeDef *pUart;
   
  if (uart == 0) {                           /* UART0 */
    NUC_GCR->PAD_REG[1] |= (1UL << 8);       /* GPIOB[2:1] = UART0 TxD, RxD  */
    NUC_CLK->APBCLK     |= (1UL << 3);       /* enable clock for UART0       */ 
  
    pUart = (NUC_UART_TypeDef *)NUC_UART0;
  } else {                                   /* UART1 */
    NUC_GCR->PAD_REG[1] |= (1UL << 9);       /* GPIOC[6:5] = UART1 TxD, RxD  */
    NUC_CLK->APBCLK     |= (1UL << 4);       /* enable clock for UART0       */ 

    pUart = (NUC_UART_TypeDef *)NUC_UART1;
  }

  pUart->FCR    = 0;                         /* disable FIFO                 */ 
  pUart->LCR    = 0x83;                      /* 8 bits, no Parity, 1 Stop bit*/
  pUart->DLL    = 11;                        /* 115200 Baud Rate @ 24 MHZ    */
  pUart->DLM    = 0;                         /* High divisor latch = 0       */
  pUart->LCR    = 0x03;                      /* DLAB = 0                     */
}


/*----------------------------------------------------------------------------
  Write character to Serial Port
 *----------------------------------------------------------------------------*/
int sendchar (int uart, int c) {
  NUC_UART_TypeDef *pUart;

  pUart = (uart == 0) ? (NUC_UART_TypeDef *)NUC_UART0 : (NUC_UART_TypeDef *)NUC_UART1;
  while (!(pUart->LSR & 0x20));
  return (pUart->THR = c);
}


/*----------------------------------------------------------------------------
  Read character from Serial Port   (blocking read)
 *----------------------------------------------------------------------------*/
int getkey (int uart) {
  NUC_UART_TypeDef *pUart;

  pUart = (uart == 0) ? (NUC_UART_TypeDef *)NUC_UART0 : (NUC_UART_TypeDef *)NUC_UART1;
  while (!(pUart->LSR & 0x01));
  return (pUart->RBR);
}
