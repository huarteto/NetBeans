/*----------------------------------------------------------------------------
 * Name:    Time.c
 * Purpose: Time Functions for 100Hz Clock Tick
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


#include <stm32f10x_lib.h>
#include "STM32_Init.h"                           // STM32 Initialization

long timeval = 0;

/* Timer Counter 0 Interrupt executes each 10ms @ 40 MHz Crystal Clock        */
void TIM1_UP_IRQHandler (void) {
  if ((TIM1->SR & 0x0001) != 0) {                 // check interrupt source
    timeval++;
    TIM1->SR &= ~(1<<0);                          // clear UIF flag
 }
} // end TIM1_UP_IRQHandler

long clock (void)  {
  return timeval;
}


void __backspace (void)  {
  ;
}
