/*----------------------------------------------------------------------------
 * Name:    Serial.c
 * Purpose: Low level serial routines
 * Note(s):
 *----------------------------------------------------------------------------
 * This file is part of the uVision/ARM development tools.
 * This software may only be used under the terms of a valid, current,
 * end user licence from KEIL for a compatible version of KEIL software
 * development tools. Nothing else gives you the right to use this software.
 *
 * This software is supplied "AS IS" without warranties of any kind.
 *
 * Copyright (c) 2009 Keil - An ARM Company. All rights reserved.
 *----------------------------------------------------------------------------*/

#include "LPC17xx.H"                    /* LPC17xx definitions                */


LPC_UART_TypeDef *pUart = (LPC_UART_TypeDef *)LPC_UART1;


/*----------------------------------------------------------------------------
  Initialize UART pins, Baudrate
 *----------------------------------------------------------------------------*/
void SER_init (void) {
   
  if ((int)pUart == (int)LPC_UART1) {     /* UART0 */
    LPC_PINCON->PINSEL0 |= (1 << 4);      /* Pin P0.2 used as TXD0 (Com0)     */
    LPC_PINCON->PINSEL0 |= (1 << 6);      /* Pin P0.3 used as RXD0 (Com0)     */
  } else {                                /* UART1 */
    LPC_PINCON->PINSEL4 |= (2 << 0);      /* Pin P2.0 used as TXD1 (Com1)     */
    LPC_PINCON->PINSEL4 |= (2 << 2);      /* Pin P2.1 used as RXD1 (Com1)     */
  }

  pUart->LCR    = 0x83;                   /* 8 bits, no Parity, 1 Stop bit    */
  pUart->DLL    = 6;                      /* 115200 Baud Rate @ 18.0 MHZ PCLK */
  pUart->FDR    = 0x85;                   /* FR 1,627, DIVADDVAL 5, MULVAL 8  */
  pUart->DLM    = 0;                      /* High divisor latch = 0           */
  pUart->LCR    = 0x03;                   /* DLAB = 0                         */
}


/*----------------------------------------------------------------------------
  Write character to Serial Port
 *----------------------------------------------------------------------------*/
int sendchar (int c) {

#ifdef __DBG_ITM
    ITM_SendChar(c);
#else
  while (!(pUart->LSR & 0x20));
  pUart->THR = c;
#endif
  return (c);
}


/*----------------------------------------------------------------------------
  Read character from Serial Port   (blocking read)
 *----------------------------------------------------------------------------*/
int getkey (void) {

  while (!(pUart->LSR & 0x01));
  return (pUart->RBR);
}
