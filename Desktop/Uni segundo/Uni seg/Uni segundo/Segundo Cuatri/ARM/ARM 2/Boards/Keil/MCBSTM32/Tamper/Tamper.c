/*----------------------------------------------------------------------------
 * Name:    Tamper.c
 * Purpose: Tamper usage for STM32
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
#include "STM32_Init.h"                           // STM32 Initialization


/*----------------------------------------------------------------------------
  TAMPER Interrupt Handler
 *----------------------------------------------------------------------------*/
void TAMPER_IRQHandler(void)
{
  if (BKP->CSR & (1<<9) ) {                       // Tamper interrupt flag
    PWR->CR  |=  (1<<8);                          // enable access to RTC, BDC registers
    BKP->CSR |=  (1<<1);                          // clear Tamper Interrupt
	BKP->CSR |=  (1<<0);                          // clear tamper Event
    PWR->CR  &= ~(1<<8);                          // disable access to RTC, BDC registers
	if ((BKP->DR1 == 0) &&
	    (BKP->DR2 == 0)   )
      GPIOB->BSRR = (1<<9);                       // LED PB9 on
    else
      GPIOB->BSRR = (1<<8);                       // LED PB8 on
    }
}


/*----------------------------------------------------------------------------
  MAIN function
 *----------------------------------------------------------------------------*/
int main (void) {

  stm32_Init ();                                  // STM32 setup

  PWR->CR  |=  (1<<8);                            // enable access to RTC, BDC registers
  BKP->DR1 = 0x55AA;                              // fill BKP_DR1 register
  BKP->DR2 = 0x33CC;                              // fill BKP_DR2 register
  PWR->CR  &= ~(1<<8);                            // disable access to RTC, BDC registers    

  while (1) {                                     // Loop forever
    ;
  } // end while
} // end main
