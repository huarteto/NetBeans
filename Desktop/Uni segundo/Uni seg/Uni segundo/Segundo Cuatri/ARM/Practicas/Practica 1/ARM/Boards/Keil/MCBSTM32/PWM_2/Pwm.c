/*----------------------------------------------------------------------------
 * Name:    Timer.c
 * Purpose: Timer usage for STM32
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


unsigned short Ccr = 0;                           // Capture , Compare value
           int Dir = 1;                           // in- decrease CCR

/*------------------------------------------------------------------------------
  Timer4 Update Interrupt Handler
 *------------------------------------------------------------------------------*/
void TIM4_IRQHandler(void)
{
  if (TIM4->SR & (1<<0)) {                        // UIF set?

    if (Ccr ==   0) Dir =  1;                     // lower limit reached ? move up
    if (Ccr == 100) Dir = -1;                     // upper limit reached ? move down
    Ccr = Ccr + Dir;   
    TIM4->CCR3 = Ccr;                             // change ccr
    TIM4->CCR4 = 100 - Ccr;                       // change ccr

    TIM4->SR &= ~(1<<0);                          // clear UIF flag
  }
} // end TIM4_IRQHandler

/*----------------------------------------------------------------------------
  MAIN function
 *----------------------------------------------------------------------------*/
int main (void) {

  stm32_Init ();                                  // STM32 setup

  while (1) {                                     // Loop forever
    ;
  } // end while
} // end main
