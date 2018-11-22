/******************************************************************************/
/* IRQ.C: IRQ Handler                                                         */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2005-2006 Keil Software. All rights reserved.                */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/

#include <71x_lib.H>


#define Offset  0x9AB   // Digital value for 0V

vs16 ADVal;
vu32 TimeVal;

extern void LED_On (u32 num);
extern void LED_Off(u32 num);


// Timer3 IRQ: Executed each 10ms
__irq void T3TIMI_IRQHandler (void) {
  u32 n;
  u16 v;

  TimeVal++;                                        // Increment Tick Counter
  if (ADVal > Offset) {                             // Calibrate Value
    v = ADVal - Offset;
  }
  if (ADVal < 0x07FF) {
    v = ADVal - Offset + 0x0FFF;
  }
  v = (5*v) >> 10;                                  // Scale the Value
  for (n = 0; n < 16; n++) {                        // Show the Value
    if (n < v) {
      LED_On (n);                                   // Turn LED On
    } else {
      LED_Off(n);                                   // Turn LED Off
    }
  }
  TIM_FlagClear(TIM3, TIM_TOF);
  EIC->IPR = 1 << T3TIMI_IRQChannel;
}

// A/D IRQ: Executed when A/D Conversion is done
__irq void ADC12_IRQHandler(void) {
  ADVal = ADC12_ConversionValue(ADC12_CHANNEL0);
  ADC12->CSR &= ~ADC12_DA0;
  EIC->IPR = 1 << ADC_IRQChannel;
}
