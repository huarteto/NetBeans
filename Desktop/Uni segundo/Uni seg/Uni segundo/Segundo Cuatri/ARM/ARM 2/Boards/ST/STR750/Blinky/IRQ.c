/******************************************************************************/
/* IRQ.C: IRQ Handler                                                         */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2005-2006 Keil Software. All rights reserved.                */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/

#include <75x_lib.H>


short AD_last;

extern void LED_On (u32 num);
extern void LED_Off(u32 num);


// Timer2 IRQ: Executed each 10ms
__irq void TIM2_UP_IRQHandler (void) {
  u32 n;
  u16 v;

  v = (5*AD_last) >> 9;                             // Scale the Value
  for (n = 0; n < 4; n++)                           // Show the Value
    if (n < v) LED_On (n);                          // Turn LED On
    else       LED_Off(n);                          // Turn LED Off

  TIM2->ISR &= ~TIM_IT_Update;                      // Clear Timer Flag
  EIC->IPR   = 1 << TIM2_UP_IRQChannel;             // Clear Int Pending Flag
}

// A/D IRQ: Executed when A/D Conversion is done
__irq void ADC_IRQHandler(void) {
  AD_last = ADC->D3;                                // Read Conversion Result
  ADC->PBR = ADC_IT_EOC;                            // Clear Conversion Flag
  EIC->IPR = 1 << ADC_IRQChannel;                   // Clear Int Pending Flag
}
