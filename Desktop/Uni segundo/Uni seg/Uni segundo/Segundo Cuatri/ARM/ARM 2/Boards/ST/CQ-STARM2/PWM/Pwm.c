/*----------------------------------------------------------------------------
 * Name:    Pwm.c
 * Purpose: Timer usage for STM32
 * Version: V1.00
 *----------------------------------------------------------------------------
 * This file is part of the uVision/ARM development tools.
 * This software may only be used under the terms of a valid, current,
 * end user licence from KEIL for a compatible version of KEIL software
 * development tools. Nothing else gives you the right to use this software.
 *
 * This software is supplied "AS IS" without warranties of any kind.
 *
 * Copyright (c) 2005-2008 Keil Software. All rights reserved.
 *----------------------------------------------------------------------------
 * History:
 *          V1.00 Initial Version
 *----------------------------------------------------------------------------*/

#include <stm32f10x_lib.h>                        // STM32F10x Library Definitions
#include "STM32_Init.h"                           // STM32 Initialization


unsigned short Ccr = 0;                           // Capture , Compare value
           int Dir = 1;                           // in- decrease CCR

/*------------------------------------------------------------------------------
  Timer3 Update Interrupt Handler
 *------------------------------------------------------------------------------*/
void TIM3_IRQHandler(void)
{
  if (TIM3->SR & (1<<0)) {                        // UIF set?

    if (Ccr ==   0) Dir =  1;                     // lower limit reached ? move up
    if (Ccr == 100) Dir = -1;                     // upper limit reached ? move down
    Ccr = Ccr + Dir;   
    TIM3->CCR1 = Ccr;                             // change ccr

    TIM3->SR &= ~(1<<0);                          // clear UIF flag
  }
} // end TIM3_IRQHandler

/*----------------------------------------------------------------------------
  MAIN function
 *----------------------------------------------------------------------------*/
int main (void) {

  stm32_Init ();                                  // STM32 setup

  while (1) {                                     // Loop forever
    ;
  } // end while
} // end main
