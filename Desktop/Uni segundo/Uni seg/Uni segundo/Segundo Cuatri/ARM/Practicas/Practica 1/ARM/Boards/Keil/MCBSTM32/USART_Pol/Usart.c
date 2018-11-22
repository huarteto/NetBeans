/*----------------------------------------------------------------------------
 * Name:    Usart.c
 * Purpose: USART usage for STM32
 * Version: V1.00
 *----------------------------------------------------------------------------
 * This file is part of the uVision/ARM development tools.
 * This software may only be used under the terms of a valid, current,
 * end user licence from KEIL for a compatible version of KEIL software
 * development tools. Nothing else gives you the right to use this software.
 *
 * Copyright (c) 2005-2007 Keil Software. All rights reserved.
 *----------------------------------------------------------------------------*/

#include <stm32f10x_lib.h>                        // STM32F10x Library Definitions
#include <stdio.h>
#include "STM32_Init.h"                           // STM32 Initialization


/*----------------------------------------------------------------------------
  SendChar
  Write character to Serial Port.
 *----------------------------------------------------------------------------*/
int SendChar (int ch)  {

  while (!(USART1->SR & USART_FLAG_TXE));
  USART1->DR = (ch & 0x1FF);

  return (ch);
}


/*----------------------------------------------------------------------------
  GetKey
  Read character to Serial Port.
 *----------------------------------------------------------------------------*/
int GetKey (void)  {

  while (!(USART1->SR & USART_FLAG_RXNE));

  return ((int)(USART1->DR & 0x1FF));
}


/*----------------------------------------------------------------------------
  MAIN function
 *----------------------------------------------------------------------------*/
int main (void) {

  stm32_Init ();                                  // STM32 setup

  printf ("Polling mode Serial I/O Example\r\n\r\n");

  while (1) {                                     // Loop forever
    unsigned char c;

    printf ("Press a key. ");
    c = getchar ();
    printf ("\r\n");
    printf ("You pressed '%c'.\r\n\r\n", c);
  } // end while
} // end main
