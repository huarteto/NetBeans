/******************************************************************************/
/* IRQ.c: IRQ Handler                                                         */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2009 Keil Software. All rights reserved.                     */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/

#include <NUC1xx.H>                     /* NUC1xx definitions                 */


unsigned short AD_last;                 /* Last converted value               */
unsigned char  clock_1s;                /* Flag activated each second         */

/* Import function for turning LEDs on or off                                 */
extern void LED_Out (unsigned int num);


/*----------------------------------------------------------------------------
  SysTick IRQ Handler
 *----------------------------------------------------------------------------*/
void SysTick_Handler (void) {           /* SysTick Interrupt Handler (10ms)   */
  static unsigned long ticks;
  static unsigned long timetick;
#ifdef LDROM
  static unsigned char leds = 0x80;
#else
  static unsigned char leds = 0x01;
#endif

  if (ticks++ >= 99) {                  /* Set Clock1s to 1 every 1 second    */
    ticks    = 0;
    clock_1s = 1;
  }

  /* Blink the LEDs depending on ADC_ConvertedValue                           */
  if (timetick++ >= (AD_last >> 8)) {
    timetick   = 0;
#ifdef APROM
   leds  ^= 0x01;
#endif
#ifdef LDROM
   leds  ^= 0x80;
#endif
#ifdef FLASH
    leds     <<= 1;
    if (leds == 0) leds = 0x01;
#endif
    LED_Out (leds);
  }

  NUC_ADC->CR   |= (1UL << 11);         /* A/D Conversion Start               */
}

/*----------------------------------------------------------------------------
 A/D IRQ Handler
 *----------------------------------------------------------------------------*/
void ADC_IRQHandler(void) {            /* called when A/D Conversion is done  */

  if (NUC_ADC->SR & (1UL << 0)) {      /* A/D Conversion End Flag set ?       */
    AD_last = (uint32_t)(NUC_ADC->DR[0] & 0x0FFF);
    NUC_ADC->SR |= (1UL << 0);         /* A/D Conversion End Flag reset       */
 }
}
