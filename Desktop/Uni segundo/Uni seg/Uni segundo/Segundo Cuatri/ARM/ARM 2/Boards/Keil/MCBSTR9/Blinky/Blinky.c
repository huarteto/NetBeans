/******************************************************************************/
/* BLINKY.C: LED Flasher for MCB-STR9 Board                                   */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2005-2006 Keil Software. All rights reserved.                */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/

#include <91x_lib.h> 
#include "LCD.h"


extern short AD_last;                     /* Last AD value read in interrupt  */

extern __irq void TIM3_IRQ_Handler(void); /* TIM3 interrupt routine           */
extern __irq void ADC_IRQ_Handler (void); /* ADC  interrupt routine           */


void wait (void)  {                       /* Wait function                    */
  int d;

  d = AD_last;                            /* Read AD_last value               */
  if (d != AD_last)                       /* Make sure that AD interrupt did  */
    d = AD_last;                          /* not interfere with value reading */

  d *= 500;
  d += 50000;                             /* Scale analog value for delay     */
                                          /* lower value -> longer delay      */
  while (d--);                            /* Only to delay for LED flashes    */
}


int main (void) {
  unsigned int i, n;
  unsigned short AD_old, AD_value;

  /* ADC Setup                                                                */
  SCU->GPIOIN[4]  |= 0x01;                /* P4.0 input  - mode 0             */
  SCU->GPIOOUT[4] &= 0xFFFC;              /* P4.0 output - mode 0             */
  GPIO4->DDR      &= 0xFE;                /* P4.0 direction - input           */
  SCU->GPIOANA    |= 0x0001;              /* P4.0 analog mode ON              */
                                          
  ADC->CR         |= 0x0002;              /* Set POR bit                      */
  for (n = 0; n < 100000; n ++);          /* Wait > 1 ms  (at 96 MHz)         */

  ADC->CR         &= 0xFFF7;              /* Clear STB bit                    */
  for (n = 0; n < 1500; n ++);            /* Wait > 15 us (at 96 MHz)         */

  ADC->CR         |= 0x0400;              /* Enable end of conversion interupt*/
  ADC->CCR         = 0x0003;              /* AD Conversion, No WDG on Ch 0    */

  SCU->GPIOOUT[7]  = 0x5555;              /* P7.0..7 output - mode 1          */
  GPIO7->DDR       = 0xFF;                /* P7.0..7 Outputs (LED Data)       */

  /* LCD Setup                                                                */
  GPIO8->DDR       = 0xFF;                /* P8.0..7 Outputs (LCD Data)       */
  GPIO9->DDR       = 0x07;                /* P9.0..2 Outputs (LCD Control)    */

  lcd_init();
  lcd_clear();
  lcd_print (" MCB-STR9 DEMO  ");
  set_cursor (0, 1);
  lcd_print ("  www.keil.com  ");

  for (i = 0; i < 200; i++) wait();       /* Wait for initial display         */

  /* Configure and enable IRQ for A/D Converter (ADC)                         */
  VIC0->VAiR[15]  = (unsigned int)ADC_IRQ_Handler; /* Setup ADC IRQ Hndl addr */
  VIC0->VCiR[15] |= 0x20;                 /* Enable the vector interrupt      */
  VIC0->VCiR[15] |= 15;                   /* Specify the interrupt number     */
  VIC0->INTER    |= (1<<15);              /* Enable ADC interrupt             */

  /* Configure and enable IRQ for Timer (TIM3)                                */
  VIC0->VAiR[7]   = (unsigned int)TIM3_IRQ_Handler;/* Setup TIM3 IRQ Hndl addr*/
  VIC0->VCiR[7]  |= 0x20;                 /* Enable the vector interrupt      */
  VIC0->VCiR[7]  |= 7;                    /* Specify the interrupt number     */
  VIC0->INTER    |= (1<<7);               /* Enable TIM3 interrupt            */

  /* Timer 3 Configuration (TIM3)                                             */
  TIM3->CNTR      = 0x0000;               /* Setup TIM3 counter register      */
  TIM3->CR2      &= 0xFF00;               /* Clear prescaler value            */
  TIM3->CR2      |= 0x000F;               /* Setup TIM3 prescaler             */
  TIM3->CR2      |= 0x2000;               /* TIM3 timer overflow intrupt en   */
  TIM3->CR1      |= 0x8000;               /* TIM3 counter enable              */

  while (1) {                             /* Loop forever                     */
    for (n = 0x01; n <= 0xFF; n <<= 1) {
      GPIO7->DR[0x3FC] = n;               /* Turn on LED                      */
      wait();                             /* Delay                            */
      AD_value = AD_last;                 /* Read AD_last value               */
      if (AD_value != AD_last)            /* Make sure that AD interrupt did  */
        AD_value = AD_last;               /* not interfere with value reading */
      AD_value /= 13;                     /* Scale to AD_Value to 0 - 78      */
      if (AD_old != AD_value)  {          /* If AD value has changed          */
        set_cursor (0, 1);
        AD_old = AD_value;
        for (i = 0; i < 16; i++)  {       /* Disp bargraph according to AD    */
          if (AD_value > 5)  {
            lcd_putchar (0x05);
            AD_value -= 5;
          }  else  {
            lcd_putchar (AD_value);
            AD_value = 0;
          }
        }
      }
    }
  }
}
