/******************************************************************************/
/* IRQ.C: IRQ Handler                                                         */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2005-2006 Keil Software. All rights reserved.                */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/

#include <73x_lib.h>                    /* STR730 library functions           */

/* Global variable to be used in main program                                 */
unsigned short AD_value;                  /* AD value                         */

/* Variable holding values for LED activation declared in module Blinky.c     */
extern const unsigned char LED7SEG[];


__irq void TIM3_IRQHandler (void) {     /* TIM3 timer interrupt routine       */
  /* Drive buzzer according to PWM, if key S2 pressed                         */
  if (GPIO_BitRead(GPIO1, GPIO_PIN_8) == 0)
    GPIO_BitWrite(GPIO2, GPIO_PIN_3, GPIO_BitRead(GPIO2, GPIO_PIN_3) ? Bit_RESET: Bit_SET);

  /* Clear TIM output compare channel B flag                                  */
  TIM_FlagClear(TIM3,TIM_FLAG_OCB);

  /* Clear interrupt pending bit                                              */
  EIC->IPR0 = 1 << TIM3_IRQChannel;
}

__irq void TIM4_IRQHandler (void) {     /* TIM4 timer interrupt routine       */
  static unsigned short DISP_active;

  /* Activate 7-seg LED display and leave it on for 1 interrupt time          */
  if (DISP_active == 0)
    /* Output high 4 bits to 7-seg LED 0                                      */
    GPIO_WordWrite(GPIO0, LED7SEG[AD_value>>6] | 0x0200);
  else
    /* Output low  4 bits to 7-seg LED 1                                      */
    GPIO_WordWrite(GPIO0, LED7SEG[(AD_value>>2) & 0x0F] | 0x0100);

  DISP_active = !DISP_active;           /* Toggle active 7-seg LED display    */

  /* Clear TIM output compare channel B flag                                  */
  TIM_FlagClear(TIM4,TIM_FLAG_OCB);

  /* Clear interrupt pending bit                                              */
  EIC->IPR0 = 1 << TIM4_IRQChannel;
}

__irq void ADC_IRQHandler(void) {       /* AD converter interrupt routine     */
  /* Store converted value to AD_value variable for main function             */
  AD_value = ADC_GetConversionValue(ADC_CHANNEL0);

  /* Clear EOC interrupt pending bit                                          */
  ADC_FlagClear(ADC_FLAG_EOC );

  /* Clear interrupt pending bit                                              */
  EIC->IPR1 = 1 << (ADC_IRQChannel-32);
}
