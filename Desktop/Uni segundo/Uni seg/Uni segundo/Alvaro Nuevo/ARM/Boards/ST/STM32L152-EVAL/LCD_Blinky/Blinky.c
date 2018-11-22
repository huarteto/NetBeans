/******************************************************************************/
/* BLINKY.C: LED Flasher                                                      */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2005-2009 Keil Software. All rights reserved.                */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/

#include "stdio.h"
#include <stm32l1xx.h>                  /* STM32L1xx.h definitions            */
#include "GLCD.h"

#define LED_NUM     4                   /* Number of user LEDs                */
const unsigned long led_mask[] = { 1UL <<  0, 1UL <<  1, 1UL << 4, 1UL << 5};

extern unsigned char ClockLEDOn;
extern unsigned char ClockLEDOff;
extern unsigned char ClockANI;
extern unsigned char ARM_Ani_16bpp[];


int main (void) {
  int ad_val   =  0, ad_avg   = 0, ad_val_  = 0xFFFF;
  int joy      =  0, joy_     = -1;
  int but      =  0, but_     = -1;
  int num      = -1; 
  int dir      =  1;
  int pic      =  0;

  SysTick_Config(SystemCoreClock/100);  /* Generate interrupt each 10 ms      */

  /* Configure the GPIO for Push Buttons                                      */
  RCC->AHBENR  |= ( 1UL << 2);          /* Enable GPIOC clock                 */
  GPIOC->MODER &= ~0x0C000000;          /* PC.13  Input mode                  */

  /* Configure the GPIO for Joystick                                          */
  RCC->AHBENR  |= ( 1UL << 4);          /* Enable GPIOE clock                 */
  GPIOE->MODER &= ~0x03FF0000;          /* PE.8..12  Input mode               */

  /* Setup GPIO for LEDs                                                      */
  RCC->AHBENR  |= (1 << 3);             /* Enable GPIOD clock                 */
  GPIOD->ODR   |=  0x00000033;          /* switch off LEDs                    */
  GPIOD->MODER &= ~0x00000F0F;
  GPIOD->MODER |=  0x00000505;          /* PB0..1, PB4..5 output mode         */

  /* Setup and initialize ADC converter                                       */
  RCC->AHBENR  |= ( 1UL <<    1);       /* Enable GPIOB clock                 */
  GPIOB->MODER |= 0x03000000;           /* PB.12 is Analog mode               */

  RCC->CR      |= ( 1UL <<    0);       /* Enable Internal high-speed clock   */
  RCC->APB2ENR |= ( 1UL <<    9);       /* Enable ADC1 clock                  */
  ADC1->SQR1    = 0x00000000;           /* Regular channel, 1 conversion      */
  ADC1->SQR2    = 0x00000000;           /* Clear register                     */
  ADC1->SQR3    = 0x00000000;           /* Clear register                     */
  ADC1->SQR4    = 0x00000000;           /* Clear register                     */
  ADC1->SQR5    = (18UL <<  0);         /* SQ1 = channel 18                   */
  ADC1->SMPR1   = 0x00000000;           /* Clear register                     */
  ADC1->SMPR2   = ( 4UL << 24);         /* Channel 18 sample time is 48 cyc   */
  ADC1->SMPR3   = 0x00000000;           /* Clear register                     */
  ADC1->CR1     = ( 1UL <<  8);         /* Scan mode on                       */

  ADC1->CR2     = ( 1UL <<  1) |
                  ( 1UL <<  0) ;        /* ADC enable                         */

  while (ADC1->SR & (1UL << 8));        /* wait until regular channel is ready*/                            
  ADC1->CR2    |= ( 1UL << 30);         /* Start A/D conversion               */ 

  GLCD_Init();                          /* Initialize graphical LCD display   */
  GLCD_Clear(White);                    /* Clear graphical LCD display        */

  GLCD_SetBackColor(Blue);
  GLCD_SetTextColor(White);
  GLCD_DisplayString(0, 0, "STM32L152-EVAL Demo ");
  GLCD_SetTextColor(Blue);
  GLCD_SetBackColor(White);
  GLCD_DisplayString(4, 0, "AD value:            ");
  GLCD_DisplayString(5, 0, "Buttons :            ");
  GLCD_DisplayString(7, 0, "Joystick:            ");


  for (;;) {                            /* Loop forever                       */
    /* Collect all input signals                                              */
    /* AD converter input                                                     */
    if (ADC1->SR & (1UL << 1)) {        /* If conversion has finished         */
      ad_avg += (ADC1->DR & 0x0FFF)<<8; /* Add AD value to averaging          */
      ad_avg ++;
      ADC1->CR2 |= ( 1UL << 30);        /* Start new conversion               */ 
      if ((ad_avg & 0xFF) == 0x10) {
        ad_val = ad_avg >> 12;
        ad_avg = 0;
      }
    }

    /* Joystick input                                                         */
    joy = 0;
    if (GPIOE->IDR & (1 << 12)) joy |= (1 << 0);  /* Joystick left            */
    if (GPIOE->IDR & (1 << 11)) joy |= (1 << 1);  /* Joystick right           */
    if (GPIOE->IDR & (1 <<  9)) joy |= (1 << 2);  /* Joystick up              */
    if (GPIOE->IDR & (1 << 10)) joy |= (1 << 3);  /* Joystick down            */
    if (GPIOE->IDR & (1 <<  8)) joy |= (1 << 4);  /* Joystick select          */
    joy ^= 0x1F;

    /* Button inputs                                                          */
    but = 0;
    if (GPIOC->IDR & (1 << 13)) but |= (1 << 0);  /* Button User (Key)        */
//    but ^= 0x01;

    /* Show all signals                                                       */
    if (ad_val ^ ad_val_) {         /* Show AD value bargraph             */
      GLCD_SetTextColor(Cyan);
      GLCD_Bargraph (144, Line4, 160, 20, (ad_val>>2));

      ad_val_ = ad_val;
    }

    if (but ^ but_) {               /* Show buttons status                */
      if (but & (1 << 0)) {
        GLCD_SetTextColor(Red);
        GLCD_DisplayString(5,  9, "Key");
      } else {
        GLCD_SetTextColor(LightGrey);
        GLCD_DisplayString(5,  9, "Key");
      }

      but_ = but;
    }

    if (joy ^ joy_) {               /* Show joystick status               */
      if (joy & (1 << 0)) {         /* check  "Left" Key */
        GLCD_SetTextColor(Red);
        GLCD_DisplayChar(7,  9, 0x80+ 9);
      } else {
        GLCD_SetTextColor(LightGrey);
        GLCD_DisplayChar(7,  9, 0x80+ 8);
      }
      if (joy & (1 << 1)) {         /* check "Right" Key */
        GLCD_SetTextColor(Red);
        GLCD_DisplayChar(7, 11, 0x80+11);
      } else {
        GLCD_SetTextColor(LightGrey);
        GLCD_DisplayChar(7, 11, 0x80+10);
      }
      if (joy & (1 << 2)) {        /* check "Up" Key */
        GLCD_SetTextColor(Red);
        GLCD_DisplayChar(6, 10, 0x80+ 5);
      } else {
        GLCD_SetTextColor(LightGrey);
        GLCD_DisplayChar(6, 10, 0x80+ 4);
      }
      if (joy & (1 << 3)) {        /* check "Down" Key */
        GLCD_SetTextColor(Red);
        GLCD_DisplayChar(8, 10, 0x80+ 7);
      } else {
        GLCD_SetTextColor(LightGrey);
        GLCD_DisplayChar(8, 10, 0x80+ 6);
      }
      if (joy & (1 << 4)) {       /* check "Sel" Key */
        GLCD_SetTextColor(Red);
        GLCD_DisplayChar(7, 10, 0x80+ 1);
      } else {
        GLCD_SetTextColor(LightGrey);
        GLCD_DisplayChar(7, 10, 0x80+ 0);
      }

      joy_ = joy;
    }

    if (ClockANI) {
      ClockANI = 0;
      if (pic++ > 8) pic = 0;
        GLCD_Bmp (99, 34, 120, 45, &ARM_Ani_16bpp[pic*(120*45*2)]);
    }

    if (ClockLEDOn) {
      ClockLEDOn = 0;
      /* Calculate 'num': 0, 1, ... , LED_NUM-1, LED_NUM-1, ... , 1, 0, 0, ...*/
      num += dir;
      if (num >= LED_NUM) { dir = -1; num = LED_NUM-1; } 
      else if   (num < 0) { dir =  1; num = 0;         }
      GPIOD->BSRRH = led_mask[num];     /* Turn LED on                        */     
    }
    if (ClockLEDOff) {
      ClockLEDOff = 0;
      GPIOD->BSRRL = led_mask[num];     /* Turn LED off                        */
    }
  }
}
