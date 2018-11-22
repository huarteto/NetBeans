/******************************************************************************/
/* IRQ.c: IRQ Handler                                                         */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2010 Keil - An ARM Company. All rights reserved.             */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/

#include <MB9BF50x.h>                   /* MB9BF50x definitions              */


unsigned short AD_last;                 /* Last converted value               */
unsigned char  clock_1s;                /* Flag activated each second         */

/* Import function for turning LEDs on or off                                 */
extern void LED_Out (unsigned int num);


/*----------------------------------------------------------------------------
  SysTick IRQ: Executed periodically
 *----------------------------------------------------------------------------*/
void SysTick_Handler (void) {           /* SysTick Interrupt Handler (10ms)   */
  static unsigned long ticks;
  static unsigned long timetick;
  static unsigned char leds = 0x01;

  if (ticks++ >= 99) {                  /* Set Clock1s to 1 every 1 second    */
    ticks    = 0;
    clock_1s = 1;
  }

  /* Blink the LEDs depending on ADC_ConvertedValue                           */
  if (timetick++ >= (AD_last >> 4)) {
    timetick   = 0;
    leds     <<= 1;
    if (leds == 0) leds = 0x01;
    LED_Out (leds);
  }

  MB9BF_ADC0->SCCR  =  (0x01);          /* Start A/D Conversion               */
}

/*----------------------------------------------------------------------------
  A/D IRQ: Executed when A/D Conversion is done
 *----------------------------------------------------------------------------*/
void ADC0_IRQHandler(void) {

  MB9BF_ADC0->ADCR &= ~0x80;            /* clear SCIF bit                     */  
  AD_last = (MB9BF_ADC0->SCFD >> 6) & 0x03FF; /* Read Conversion Result       */
  MB9BF_ADC0->SCCR |= (0x10);           /* clear FIFO                         */
}
