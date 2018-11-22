/*----------------------------------------------------------------------------
 * Name:    Serial.c
 * Purpose: Low level serial routines for STM32
 * Version: V1.00
 * Note(s): 
 *----------------------------------------------------------------------------
 * This file is part of the uVision/ARM development tools.
 * This software may only be used under the terms of a valid, current,
 * end user licence from KEIL for a compatible version of KEIL software
 * development tools. Nothing else gives you the right to use this software.
 *
 * This software is supplied "AS IS" without warranties of any kind.
 *
 * Copyright (c) 2008 Keil - An ARM Company. All rights reserved.
 *----------------------------------------------------------------------------
 * History:
 *          V1.00 Initial Version
 *----------------------------------------------------------------------------*/

#include <stm32f10x_lib.h>                        // STM32F10x Library Definitions

#define USARTx   USART1                           // USART1 is used

/*----------------------------------------------------------------------------
  Write character to Serial Port
 *----------------------------------------------------------------------------*/
int sendchar (int c) {

  if (c == '\n')  {
    while (!(USARTx->SR & USART_FLAG_TXE));
    USARTx->DR = 0x0D;
  }
  while (!(USARTx->SR & USART_FLAG_TXE));
  USARTx->DR = (c & 0x1FF);

  return (c);
}


/*----------------------------------------------------------------------------
  Read character from Serial Port   (blocking read)
 *----------------------------------------------------------------------------*/
int getkey (void) {

  while (!(USARTx->SR & USART_FLAG_RXNE));

  return ((int)(USARTx->DR & 0x1FF));
}
