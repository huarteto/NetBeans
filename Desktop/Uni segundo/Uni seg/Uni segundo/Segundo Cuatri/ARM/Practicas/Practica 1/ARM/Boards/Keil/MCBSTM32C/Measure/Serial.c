/*----------------------------------------------------------------------------
 * Name:    Serial.c
 * Purpose: Low level serial routines for STM32
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

#include <stm32f10x_cl.h>

/*----------------------------------------------------------------------------
 *       init_serial:  Initialize Serial Interface
 *---------------------------------------------------------------------------*/
void init_serial (void) {
  int i;

  /* Configure UART2 for 115200 baud                                         */
  RCC->APB2ENR |=  1 <<  0;            /* Enable AFIO clock                  */
  RCC->APB2ENR |=  1 <<  5;            /* Enable GPIOD clock                 */
  AFIO->MAPR |= 0x00000008;            /* Configure used Pins                */
  GPIOD->CRL &= 0xF00FFFFF;
  GPIOD->CRL |= 0x04B00000;

  RCC->APB1ENR |= 0x00020000;          /* Enable USART#2 clock               */
  USART2->BRR = 0x0135;                /* Configure 115200 baud,             */
  USART2->CR3 = 0x0000;                /*           8 bit, 1 stop bit,       */     
  USART2->CR2 = 0x0000;                /*           no parity                */
  for (i = 0; i < 0x1000; i++) __NOP();/* avoid unwanted output              */
  USART2->CR1 = 0x200C;
}

/*----------------------------------------------------------------------------
  Write character to Serial Port
 *----------------------------------------------------------------------------*/
int sendchar (int c) {

  if (c == '\n')  {
    while (!(USART2->SR & 0x0080));
    USART2->DR = 0x0D;
  }
   while (!(USART2->SR & 0x0080));
  USART2->DR = (c & 0x1FF);

  return (c);
}


/*----------------------------------------------------------------------------
  Read character from Serial Port   (blocking read)
 *----------------------------------------------------------------------------*/
int getkey (void) {

   while (!(USART2->SR & 0x0020));
   return (USART2->DR);
}
