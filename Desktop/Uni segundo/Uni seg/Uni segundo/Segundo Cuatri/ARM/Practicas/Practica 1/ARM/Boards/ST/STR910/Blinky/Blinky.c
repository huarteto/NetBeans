/******************************************************************************/
/* BLINKY.C: LED Flasher for STR910 Board                                     */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2005-2006 Keil Software. All rights reserved.                */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/

#include <91x_lib.h>
#include "GLCD.h"

unsigned short AD_value;


void wait (void)  {                          /* Wait function                 */
  int  d;
                                           
  ADC->CR = ADC->CR | 0x0183;                /* Set STR bit (Start Conversion)*/
                                             /* on Channel 6                  */
  while ((ADC->CR & 0x8000) != 0x8000);      /* Wait for AD to finish         */

  AD_value = ADC->DR6 & 0x03FF;              /* AD value for use in main func */
  d = 50000 + (~ADC->DR6 & 0x03FF) * 100;    /* Scale analog value for delay  */
                                             /* lower value -> longer delay   */

  while (d--);                               /* Only to delay for LED flashes */
}


int main (void) {
  unsigned int i, n;
  unsigned short AD_old;

  /* ADC Setup                                                                */
  SCU->GPIOIN[4]  = SCU->GPIOIN[4] | 0x40;   /* P4.6 input  - mode 0          */
  SCU->GPIOOUT[4] = SCU->GPIOOUT[4] & 0xCFFF;/* P4.6 output - mode 0          */
  GPIO4->DDR      = GPIO4->DDR & 0xBF;       /* P4.6 direction - input        */
  SCU->GPIOANA    = SCU->GPIOANA | 0x0040;   /* P4.6 analog mode ON           */

  ADC->CR = ADC->CR | 0x0002;                /* Set POR bit                   */
  for (n = 0; n < 100000; n ++);             /* Wait > 1 ms                   */

  ADC->CR = ADC->CR & 0xFFF7;                /* Clear STB bit                 */
  for (n = 0; n < 1500; n ++);               /* Wait > 15 us                  */

  ADC->CCR = 0x3000;                         /* AD Conversion, No WDG on Ch 6 */

  /* LCD Setup                                                                */
  GPIO8->DDR = 0xFF;                         /* P8.0..7 Outputs (LCD Data)    */
  GPIO9->DDR = 0xFF;                         /* P9.0..7 Outputs (LCD+LED)     */

  glcd_init();
  glcd_clear();

  glcd_print(0, 0, 0, "This is a Keil DEMO ");
  glcd_print(0, 1, 0, "for the STR910 Board");
  glcd_print(0, 2, 0, "   www.keil.com     ");
  glcd_print(0, 3, 0, "AD:                 ");

  while (1) {                                /* Loop forever                  */
    for (n = 0x01; n <= 0x08; n <<= 1) {
      GPIO9->DR[0x03C] = ~n;                 /* Turn on LED                   */
      wait();                                /* Delay                         */
      AD_value >>= 6;
      if (AD_old != AD_value)  {             /* Disp bargraph according to AD */
        for (i = 0; i < 16; i++)
          if (i < AD_value)
            glcd_print(i+4, 3, 0, "\1");
          else
            glcd_print(i+4, 3, 0, " ");
      }
      AD_old = AD_value;
    }
  }
}
