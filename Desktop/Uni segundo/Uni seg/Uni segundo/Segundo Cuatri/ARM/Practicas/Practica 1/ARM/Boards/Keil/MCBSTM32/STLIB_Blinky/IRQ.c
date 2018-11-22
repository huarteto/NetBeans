/******************************************************************************/
/* IRQ.C: IRQ Handler                                                         */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2005-2007 Keil Software. All rights reserved.                */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/

#include <stm32f10x_lib.h>              /* STM32F10x Library Definitions      */


unsigned char Clock1s;

extern unsigned short int ADC_ConvertedValue;


/* SysTick interrupt happens every 10 ms                                      */
void SysTick_Handler (void) {
  static unsigned long ticks;
  static unsigned long timetick;
  static unsigned char leds = 0x01;

  if (ticks++ >= 99) {                  /* Set Clock1s to 1 every 1 second    */
    ticks   = 0;
    Clock1s = 1;
  }

  /* Activate LEDs with shift                                                 */
  if (timetick++ >= (ADC_ConvertedValue>>8)) {
    timetick   = 0;
    leds     <<= 1;
    if (leds == 0) leds = 0x01;
    GPIOB->ODR = (GPIOB->ODR & 0xFFFF00FF) | (leds << 8);
  }
}
