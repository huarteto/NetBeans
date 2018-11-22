/******************************************************************************/
/* BLINKY.C: LED Flasher                                                      */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2005-2009 Keil Software. All rights reserved.                */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/

#include <stm32f10x_cl.h>

#define LED_NUM     8                   /* Number of user LEDs                */

const long led_mask[] = { 1<<15, 1<<14, 1<<13, 1<<12, 1<<11, 1<<10, 1<<9, 1<<8 };


int main (void) {
  int AD_val, i;
  int num = -1; 
  int dir =  1;

  SystemInit();

  /* Setup GPIO for LEDs                                                      */
  RCC->APB2ENR |=  1 <<  6;             /* Enable GPIOE clock                 */
  GPIOE->CRH    = 0x33333333;           /* Configure the GPIO for LEDs        */

  /* Setup and initialize ADC converter                                       */
  RCC->APB2ENR |=  1 <<  9;             /* Enable ADC1 clock                  */
  GPIOC->CRL   &= 0xFFF0FFFF;           /* Configure PC4 as ADC.14 input      */
  ADC1->SQR1    = 0x00000000;           /* Regular channel 1 conversion       */
  ADC1->SQR2    = 0x00000000;           /* Clear register                     */
  ADC1->SQR3    = 14 <<  0;             /* SQ1 = channel 14                   */
  ADC1->SMPR1   =  5 << 12;             /* Channel 14 sample time is 55.5 cyc */
  ADC1->SMPR2   = 0x00000000;           /* Clear register                     */
  ADC1->CR1     =  1 <<  8;             /* Scan mode on                       */
  ADC1->CR2     = (1 << 20) |           /* Enable external trigger            */
                  (7 << 17) |           /* EXTSEL = SWSTART                   */
                  (1 <<  1) |           /* Continuous conversion              */
                  (1 <<  0) ;           /* ADC enable                         */
  ADC1->CR2    |=  1 <<  3;             /* Initialize calibration registers   */
  while (ADC1->CR2 & (1 << 3));         /* Wait for initialization to finish  */
  ADC1->CR2    |=  1 <<  2;             /* Start calibration                  */
  while (ADC1->CR2 & (1 << 2));         /* Wait for calibration to finish     */
  ADC1->CR2    |=  1 << 22;             /* Start first conversion             */ 

  for (;;) {                            /* Loop forever                       */
    if (ADC1->SR & (1 << 1)) {          /* If conversion has finished         */
      AD_val = ADC1->DR & 0x0FFF;       /* Read AD converted value            */
      ADC1->CR2 |= 1 << 22;             /* Start new conversion               */ 
    }

    /* Calculate 'num': 0, 1, ... , LED_NUM-1, LED_NUM-1, ... , 1, 0, 0, ...  */
    num += dir;
    if (num >= LED_NUM) { dir = -1; num = LED_NUM-1; } 
    else if   (num < 0) { dir =  1; num = 0;         }

    GPIOE->BSRR = led_mask[num];        /* Turn LED on                        */
    for (i = 0; i < ((AD_val << 8) + 100000); i++);
    GPIOE->BSRR = led_mask[num] << 16;  /* Turn LED off                       */
  }
}
