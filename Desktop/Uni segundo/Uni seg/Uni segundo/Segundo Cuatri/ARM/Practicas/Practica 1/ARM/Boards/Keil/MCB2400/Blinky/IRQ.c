/******************************************************************************/
/* IRQ.C: IRQ Handler                                                         */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2005-2007 Keil Software. All rights reserved.                */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/

#include <LPC23xx.H>                    /* LPC23xx/LPC24xx definitions        */


unsigned int leds;                      /* Value to be displayed on leds      */
short AD_last;                          /* Last converted value               */
unsigned char clock_1s;                 /* Flag activated each second         */

/* Import function for outputing value to LEDs                                */
extern void LED_Out(unsigned int value);


/* Timer0 IRQ: Executed periodically                                          */
__irq void T0_IRQHandler (void) {
  static int clk_cntr;

  clk_cntr++;
  if (clk_cntr >= 1000) {
    clk_cntr = 0;
    clock_1s = 1;                       /* Activate flag every 1 second       */
  }

  /* Prepare LED values that will be displayed                                */
  if      (AD_last > (4*1024/5)) leds = 0x0F;
  else if (AD_last > (3*1024/5)) leds = 0x07;
  else if (AD_last > (2*1024/5)) leds = 0x03;
  else if (AD_last > (1*1024/5)) leds = 0x01;
  else                           leds = 0x00;

  AD0CR |= 0x01000000;                  /* Start A/D Conversion               */

  T0IR        = 1;                      /* Clear interrupt flag               */
  VICVectAddr = 0;                      /* Acknowledge Interrupt              */
}

/* A/D IRQ: Executed when A/D Conversion is done                              */
__irq void ADC_IRQHandler(void) {

  AD_last = (AD0DR0 >> 6) & 0x3FF;      /* Read Conversion Result             */

  VICVectAddr = 0;                      /* Acknowledge Interrupt              */
}
