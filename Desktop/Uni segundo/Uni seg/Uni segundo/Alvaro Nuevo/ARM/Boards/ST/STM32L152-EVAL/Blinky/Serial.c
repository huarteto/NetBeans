/******************************************************************************/
/* Serial.c: Low Level Serial Routines                                        */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2010 Keil - An ARM Company. All rights reserved.             */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/

#include <stm32l1xx.h>                  /* STM32L1xx.h definitions            */

/*----------------------------------------------------------------------------
  Initialize UART pins, Baudrate
 *----------------------------------------------------------------------------*/
void SER_init (void) {
#ifdef __DBG_ITM
  /* initialization not necessary */
#endif
 
#ifdef __USART2                         /* Configure UART2 for 115200 baud   */
  RCC->AHBENR   |=  (1 << 3);           /* Enable GPIOD clock                */

  /* Configure PD.06 to USART2_RX (AF07), PD.05 to USART2_TX (AF07) */
  GPIOD->AFR[0] &= ~((15UL << 4* 6) | (15UL << 4* 5) );   
  GPIOD->AFR[0] |=  (( 7UL << 4* 6) | ( 7UL << 4* 5) );      
  GPIOD->MODER  &= ~(( 3UL << 2* 6) | ( 3UL << 2* 5) );   
  GPIOD->MODER  |=  (( 2UL << 2* 6) | ( 2UL << 2* 5) );

  RCC->APB1ENR |=  (1 << 17);           /* Enable USART#2 clock              */
  USART2->BRR = 0x0110;                 /* Configure 115200 baud,            */
  USART2->CR3 = 0x0000;                 /*                                   */     
  USART2->CR2 = 0x0000;                 /* 1 stop bit                        */
  USART2->CR1 = 0x200C;                 /* usart enable, 8 bit, no parity    */
#endif
 
#ifdef __USART3                         /* Configure UART3 for 115200 baud   */
  RCC->AHBENR   |=  (1 << 2);           /* Enable GPIOC clock                */

  /* Configure PC.11 to USART3_RX (AF07), PC.10 to USART3_TX (AF07) */
  GPIOC->AFR[1] &= ~((15UL << 4* 3) | (15UL << 4* 2) );   
  GPIOC->AFR[1] |=  (( 7UL << 4* 3) | ( 7UL << 4* 2) );      
  GPIOC->MODER  &= ~(( 3UL << 2*11) | ( 3UL << 2*10) );   
  GPIOC->MODER  |=  (( 2UL << 2*11) | ( 2UL << 2*10) );

  RCC->APB1ENR |=  (1 << 18);           /* Enable USART#3 clock              */
  USART3->BRR = 0x0110;                 /* Configure 115200 baud,            */
  USART3->CR3 = 0x0000;                 /*                                   */     
  USART3->CR2 = 0x0000;                 /* 1 stop bit                        */
  USART3->CR1 = 0x200C;                 /* usart enable, 8 bit, no parity    */
#endif
}


/*----------------------------------------------------------------------------
  Write character to Serial Port
 *----------------------------------------------------------------------------*/
int sendchar (int c) {

#ifdef __DBG_ITM
    ITM_SendChar(c);
#endif
#ifdef __USART2
   while (!(USART2->SR & 0x0080));
  USART2->DR = (c & 0x1FF);
#endif
#ifdef __USART3
   while (!(USART3->SR & 0x0080));
  USART3->DR = (c & 0x1FF);
#endif
  return (c);
}


/*----------------------------------------------------------------------------
  Read character from Serial Port   (blocking read)
 *----------------------------------------------------------------------------*/
int getkey (void) {
#ifdef __DBG_ITM
  return(-1);
#endif
#ifdef __USART2
  while (!(USART2->SR & 0x0020));
  return (USART2->DR);
#endif
#ifdef __USART3
  while (!(USART3->SR & 0x0020));
  return (USART3->DR);
#endif
}
