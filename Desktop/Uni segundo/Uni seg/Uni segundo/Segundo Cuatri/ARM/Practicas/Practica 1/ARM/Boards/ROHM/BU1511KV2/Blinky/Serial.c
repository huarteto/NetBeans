/******************************************************************************/
/* Serial.c: Low Level Serial Routines                                        */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2009-2010 KEIL - An ARM Company. All rights reserved.        */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/

#include "BU1511KV2.h"                    /* BU1511KV2 settings               */

/*
   Note:
        UART1 corrupts character if configured for 115200 Baud. 
          I can not find the reason for this fault.
        UART2 works correct      if configured for 115200 Baud.     
 */

/*----------------------------------------------------------------------------
  Initialize UART pins, Baudrate
 *----------------------------------------------------------------------------*/
void SER_init (int uart) {
  ROHM_UART_TypeDef *pUart;
  uint32_t i;
   
  if (uart == 1) {                        /* UART1 */
    /* configuring pins not necessary */

    pUart = (ROHM_UART_TypeDef *)ROHM_UART1;
  } else {                                /* UART2 */
    /* configuring pins not necessary */

    pUart = (ROHM_UART_TypeDef *)ROHM_UART2;
  }

  pUart->FCR    = 0;                      /* disable FIFO                     */ 
  pUart->LCR    = 0x83;                   /* 8 bits, no Parity, 1 Stop bit    */
  pUart->DLL    = 22;                     /* 115200 Baud Rate @ 40.5 MHZ PCLK */
  pUart->DLH    = 0;                      /* High divisor latch = 0           */
  pUart->LCR    = 0x03;                   /* DLAB = 0                         */

  for (i = 0; i < 0x100; i++) __nop();    /* delay loop (see documentation)   */
}


/*----------------------------------------------------------------------------
  Write character to Serial Port
 *----------------------------------------------------------------------------*/
int sendchar (int uart, int c) {
  ROHM_UART_TypeDef *pUart;

  pUart = (uart == 1) ? (ROHM_UART_TypeDef *)ROHM_UART1 : (ROHM_UART_TypeDef *)ROHM_UART2;
  while (!(pUart->LSR & 0x20));
  return (pUart->THR = c);
}


/*----------------------------------------------------------------------------
  Read character from Serial Port   (blocking read)
 *----------------------------------------------------------------------------*/
int getkey (int uart) {
  ROHM_UART_TypeDef *pUart;

  pUart = (uart == 1) ? (ROHM_UART_TypeDef *)ROHM_UART1 : (ROHM_UART_TypeDef *)ROHM_UART2;
  while (!(pUart->LSR & 0x01));
  return (pUart->RBR);
}
