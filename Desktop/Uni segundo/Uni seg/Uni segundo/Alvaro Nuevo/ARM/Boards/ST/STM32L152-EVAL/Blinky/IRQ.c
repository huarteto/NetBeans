/******************************************************************************/
/* IRQ.c: IRQ Handler                                                         */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2010 Keil - An ARM Company. All rights reserved.             */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/

#include <stm32l1xx.h>                  /* STM32L1xx.h definitions            */


unsigned short AD_last;                 /* Last converted value               */
unsigned int   AD_avg = 0;
unsigned char  clock_1s;                /* Flag activated each second         */
unsigned char  clock_50ms;              /* Flag activated each 50  ms         */

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

  switch (ticks++) {
    case 49:
      clock_50ms = 1;
      break;
    case 99:
      ticks      = 0;
      clock_50ms = 1;
      clock_1s   = 1;
      break;
  }

  /* Blink the LEDs depending on ADC_ConvertedValue                           */
  if (timetick++ >= (AD_last >> 8)) {
    timetick   = 0;
    leds     <<= 1;
    if (leds == 0) leds = 0x01;
    LED_Out (leds);
  }

  if ((ADC1->SR & ((1 << 6) | (1 << 8))) == (1 << 6)) {
    ADC1->CR2    |=  (1 << 30);         /* Start A/D conversion               */ 
  }
}


#ifdef __USE_IRQ
/*----------------------------------------------------------------------------
  calculate an average over 16 AD values
 *----------------------------------------------------------------------------*/
static unsigned short CalcAverage (unsigned short val)  {
  static unsigned short idx      =  0;
  static unsigned short aval[32] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                                    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
  static unsigned int   sum      =  0;

  sum = sum - aval[idx] + val;
  aval[idx] = val;         
  idx = (idx +1 ) & 0x1F;

  return (sum >> 5);
}


/*----------------------------------------------------------------------------
  A/D IRQ: Executed when A/D Conversion is done
 *----------------------------------------------------------------------------*/
void ADC1_IRQHandler(void) {

  if (ADC1->SR & (1<<1)) {              /* ADC1 EOC interrupt?                */
//    AD_last = ADC1->DR;
    AD_last  = CalcAverage (ADC1->DR);  /* calculate average                  */

    ADC1->SR &= ~(1<<1);                /* clear EOC interrupt                */
  }

}
#endif //__USE_IRQ
