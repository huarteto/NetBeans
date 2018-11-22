/******************************************************************************/
/* IRQ.c: IRQ Handler                                                         */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2010 Keil - An ARM Company. All rights reserved.             */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/

#include "stm32f10x.h"                  /* STM32F10x.h definitions            */


unsigned short AD_last;                 /* Last converted value               */
unsigned char  AD_done = 0;             /* AD conversion done flag            */
unsigned char  clock_1s;                /* Flag activated each second         */

/* Import function for turning LEDs on or off                                 */
extern void LED_Out (unsigned int num);


/*----------------------------------------------------------------------------
  Systick Interrupt Handler
  SysTick interrupt happens every 10 ms
 *----------------------------------------------------------------------------*/
void SysTick_Handler (void) {           /* SysTick Interrupt Handler (10ms)   */
  static unsigned long ticks = 0;
  static unsigned long timetick;
  static unsigned char leds = 0x01;

  if (ticks++ >= 99) {                  /* Set Clock1s to 1 every 1 second    */
    ticks    = 0;
    clock_1s = 1;
  }

  /* Blink the LEDs depending on ADC_ConvertedValue                           */
  if (timetick++ >= (AD_last >> 8)) {
    timetick   = 0;
    leds     <<= 1;
    if (leds == 0x10) leds = 0x01;
    LED_Out (leds);
  }

    ADC1->CR2    |=  (1UL << 22);       /* Start A/D conversion               */ 
}



/*----------------------------------------------------------------------------
  DMA IRQ: Executed when a transfer is completet
 *----------------------------------------------------------------------------*/
void DMA1_Channel1_IRQHandler(void) {

  if (DMA1->ISR & (1 << 1)) {            /* TCIF interrupt?                   */
    AD_done = 1;

    DMA1->IFCR  = (1 << 1);              /* clear TCIF interrupt              */
  }

}



#ifdef __USE_IRQ
/*----------------------------------------------------------------------------
  A/D IRQ: Executed when A/D Conversion is done
 *----------------------------------------------------------------------------*/
void ADC1_IRQHandler(void) {

  if (ADC1->SR & (1 << 1)) {            /* ADC1 EOC interrupt?                */
    AD_last = ADC1->DR;
    AD_done = 1;

    ADC1->SR &= ~(1 << 1);              /* clear EOC interrupt                */
  }

}
#endif //__USE_IRQ
