/******************************************************************************/
/* Blinky.c: LED Flasher                                                      */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2010 Keil - An ARM Company. All rights reserved.             */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/

#include <stdio.h>
#include "stm32f10x.h"                  /* STM32F10x.h definitions            */
#include "GLCD.h"

#define __FI        1                   /* Font index 16x24                   */

#define LED_NUM     8                   /* Number of user LEDs                */
const unsigned long led_mask[] = { 1UL <<  8, 1UL <<  9, 1UL << 10, 1UL << 11,
                                   1UL << 12, 1UL << 13, 1UL << 14, 1UL << 15};

               char text[40];

/* Import external function from Serial.c file                                */
extern void SER_init (void);

/* Import external variables from IRQ.c file                                  */
extern unsigned short AD_last;
extern unsigned char  AD_done;
extern unsigned char  clock_1s;

/* variable to trace in LogicAnalyzer (should not read to often)              */
       volatile unsigned short AD_dbg, AD_print;          


/*----------------------------------------------------------------------------
  note: 
  set __USE_LCD in "options for target - C/C++ - Define" to enable Output on LCD
  set __USE_IRQ in "options for target - C/C++ - Define" to enable ADC in IRQ mode
                                                        default is ADC in DMA mode
 *----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------
  Function that initializes ADC
 *----------------------------------------------------------------------------*/
void ADC_init (void) {

  RCC->APB2ENR |= ( 1UL <<  4);         /* enable periperal clock for GPIOC   */
  GPIOC->CRL &= ~0x000F0000;            /* Configure PC4 as ADC.14 input      */

#ifndef __USE_IRQ
  /* DMA1 Channel1 configuration ---------------------------------------------*/
  RCC->AHBENR |= ( 1UL <<  0);          /* enable periperal clock for DMA     */

  DMA1_Channel1->CMAR  = (u32)&AD_last;    /* set channel1 memory address     */
  DMA1_Channel1->CPAR  = (u32)&(ADC1->DR); /* set channel1 peripheral address */
  DMA1_Channel1->CNDTR = 1;             /* transmit 1 word                    */
  DMA1_Channel1->CCR   = 0x00002522;    /* configure DMA channel              */
  NVIC_EnableIRQ(DMA1_Channel1_IRQn);   /* enable DMA1 Channel1 Interrupt     */
  DMA1_Channel1->CCR  |= (1 << 0);      /* DMA Channel 1 enable               */
#endif

  /* Setup and initialize ADC converter                                       */
  RCC->APB2ENR |= ( 1UL <<  9);         /* enable periperal clock for ADC1    */

  ADC1->SQR1    =  0;                   /* Regular channel 1 conversion       */
  ADC1->SQR2    =  0;                   /* Clear register                     */
  ADC1->SQR3    = (14UL <<  0);         /* SQ1 = channel 14                   */
  ADC1->SMPR1   = ( 5UL << 12);         /* sample time channel 14 55,5 cycles */
  ADC1->CR1     = ( 1UL <<  8);         /* Scan mode on                       */
  ADC1->CR2     = ( 7UL << 17)|         /* select SWSTART                     */
                  ( 1UL << 20) ;        /* enable ext. Trigger                */

#ifndef __USE_IRQ
  ADC1->CR2    |= ( 1UL <<  8);         /* DMA mode enable                    */
#else
  ADC1->CR1    |= ( 1UL <<  5);         /* enable for EOC Interrupt           */
  NVIC_EnableIRQ(ADC1_2_IRQn);          /* enable ADC Interrupt               */
#endif

  ADC1->CR2    |= ( 1UL <<  0);         /* ADC enable                         */

  ADC1->CR2    |=  1 <<  3;             /* Initialize calibration registers   */
  while (ADC1->CR2 & (1 << 3));         /* Wait for initialization to finish  */
  ADC1->CR2    |=  1 <<  2;             /* Start calibration                  */
  while (ADC1->CR2 & (1 << 2));         /* Wait for calibration to finish     */
}


/*----------------------------------------------------------------------------
  Function that initializes User Buttons
 *----------------------------------------------------------------------------*/
void BUT_init(void) {
  RCC->APB2ENR |= (1UL << 8);           /* Enable GPIOG clock                 */

  GPIOG->CRH   &= ~0x0000000F;          /* Configure the GPIO for Button      */
  GPIOG->CRH   |=  0x00000004;
}

/*----------------------------------------------------------------------------
  Function that initializes Joystick
 *----------------------------------------------------------------------------*/
void JOY_init(void) {
  RCC->APB2ENR |= (1UL << 8);           /* Enable GPIOG clock                 */
  RCC->APB2ENR |= (1UL << 5);           /* Enable GPIOD clock                 */

  GPIOG->CRL   &= ~0xF0000000;          /* Configure the GPIO for Joystick    */
  GPIOG->CRL   |=  0x40000000;
  GPIOG->CRH   &= ~0xFFF00000;
  GPIOG->CRH   |=  0x44400000;

  GPIOD->CRL   &= ~0x0000F000;          /* Configure the GPIO for Joystick    */
  GPIOD->CRL   |=  0x00004000;
}

/*----------------------------------------------------------------------------
  Function that initializes LEDs
 *----------------------------------------------------------------------------*/
void LED_init(void) {
  RCC->APB2ENR |= (1UL << 3);           /* Enable GPIOB clock                 */

  GPIOB->ODR   &= ~0x0000FF00;          /* switch off LEDs                    */
  GPIOB->CRH   &= ~0xFFFFFFFF;          /* Configure the GPIO for LEDs        */
  GPIOB->CRH   |=  0x33333333;
}

/*----------------------------------------------------------------------------
  Function that turns on requested LED
 *----------------------------------------------------------------------------*/
void LED_On (unsigned int num) {

  GPIOB->BSRR = led_mask[num];
}

/*----------------------------------------------------------------------------
  Function that turns off requested LED
 *----------------------------------------------------------------------------*/
void LED_Off (unsigned int num) {

  GPIOB->BRR = led_mask[num];
}

/*----------------------------------------------------------------------------
  Function that outputs value to LEDs
 *----------------------------------------------------------------------------*/
void LED_Out(unsigned int value) {
  int i;

  for (i = 0; i < LED_NUM; i++) {
    if (value & (1<<i)) {
      LED_On (i);
    } else {
      LED_Off(i);
    }
  }
}


/*----------------------------------------------------------------------------
  Main Program
 *----------------------------------------------------------------------------*/
int main (void) {
  int ad_val   =  0, ad_avg   = 0, ad_val_  = 0xFFFF;
  int joy      =  0, joy_     = -1;
  int but      =  0, but_     = -1;

  SysTick_Config(SystemCoreClock/100);  /* Generate interrupt each 10 ms      */

  LED_init();                           /* LED Initialization                 */
  BUT_init();                           /* User Button Initialization         */
  JOY_init();                           /* Joystick Initialization            */
  SER_init();                           /* UART Initialization                */
  ADC_init();                           /* ADC Initialization                 */

#ifdef __USE_LCD
  GLCD_Init();                          /* Initialize graphical LCD display   */

  GLCD_Clear(White);                    /* Clear graphical LCD display        */
  GLCD_SetBackColor(Blue);
  GLCD_SetTextColor(White);
  GLCD_DisplayString(0, 0, __FI, "STM32100E-EVAL Demo ");
  GLCD_DisplayString(1, 0, __FI, "       Blinky       ");
  GLCD_DisplayString(2, 0, __FI, "    www.keil.com    ");
  GLCD_SetBackColor(White);
  GLCD_SetTextColor(Blue);
  GLCD_DisplayString(4, 0, __FI, "AD value:            ");
  GLCD_DisplayString(6, 0, __FI, "Buttons :            ");
  GLCD_DisplayString(8, 0, __FI, "Joystick:            ");
#endif // __USE_LCD

  while (1) {                           /* Loop forever                       */
    /* Collect all input signals                                              */

    /* AD converter input                                                     */
    if (AD_done) {                      /* If conversion has finished         */
      AD_done = 0;
                                        /* Ad value is red via IRQ or DMA     */
      ad_avg += AD_last << 8;             /* Add AD value to averaging          */
      ad_avg ++;
      if ((ad_avg & 0xFF) == 0x10) {
        ad_val = ad_avg >> 12;
        ad_avg = 0;
      }
    }

    /* Joystick input                                                         */
    joy = 0;
    if (GPIOG->IDR & (1 << 14)) joy |= (1 << 0);  /* Joystick left            */
    if (GPIOG->IDR & (1 << 13)) joy |= (1 << 1);  /* Joystick right           */
    if (GPIOG->IDR & (1 << 15)) joy |= (1 << 2);  /* Joystick up              */
    if (GPIOD->IDR & (1 <<  3)) joy |= (1 << 3);  /* Joystick down            */
    if (GPIOG->IDR & (1 <<  7)) joy |= (1 << 4);  /* Joystick select          */
    joy ^= 0x1F;

    /* Button inputs                                                          */
    but = 0;
    if (GPIOG->IDR & (1 <<  8)) but |= (1 << 0);  /* Button User (User)       */
    but ^= 0x01;

    /* Show all signals                                                       */
    if (ad_val ^ ad_val_) {             /* Show AD value bargraph             */
      sprintf(text, "0x%04X", ad_val);
#ifdef __USE_LCD
      GLCD_SetTextColor(Red);
      GLCD_DisplayString(4,  9, __FI,  (unsigned char *)text);
      GLCD_Bargraph (144, Line5, 176, 20, (ad_val>>2));
#endif // __USE_LCD

      AD_print = ad_val;                /* Get unscaled value for printout    */
      AD_dbg   = ad_val;

      ad_val_ = ad_val;
    }

    if (but ^ but_) {                    /* Show buttons status                */
#ifdef __USE_LCD
      if (but & (1 << 0)) {
        GLCD_SetTextColor(Red);
        GLCD_DisplayString(6,  9, __FI, "User");
      } else {
        GLCD_SetTextColor(LightGrey);
        GLCD_DisplayString(6,  9, __FI, "User");
      }
#endif // __USE_LCD

      but_ = but;
    }

    if (joy ^ joy_) {                    /* Show joystick status               */
#ifdef __USE_LCD
      if (joy & (1 << 0)) {              /* check  "Left" Key */
        GLCD_SetTextColor(Red);
        GLCD_DisplayChar(8,  9, __FI, 0x80+ 9);
      } else {
        GLCD_SetTextColor(LightGrey);
        GLCD_DisplayChar(8,  9, __FI, 0x80+ 8);
      }
      if (joy & (1 << 1)) {              /* check "Right" Key */
        GLCD_SetTextColor(Red);
        GLCD_DisplayChar(8, 11, __FI, 0x80+11);
      } else {
        GLCD_SetTextColor(LightGrey);
        GLCD_DisplayChar(8, 11, __FI, 0x80+10);
      }
      if (joy & (1 << 2)) {             /* check "Up" Key */
        GLCD_SetTextColor(Red);
        GLCD_DisplayChar(7, 10, __FI, 0x80+ 5);
      } else {
        GLCD_SetTextColor(LightGrey);
        GLCD_DisplayChar(7, 10, __FI, 0x80+ 4);
      }
      if (joy & (1 << 3)) {             /* check "Down" Key */
        GLCD_SetTextColor(Red);
        GLCD_DisplayChar(9, 10, __FI, 0x80+ 7);
      } else {
        GLCD_SetTextColor(LightGrey);
        GLCD_DisplayChar(9, 10, __FI, 0x80+ 6);
      }
      if (joy & (1 << 4)) {            /* check "Sel" Key */
        GLCD_SetTextColor(Red);
        GLCD_DisplayChar(8, 10, __FI, 0x80+ 1);
      } else {
        GLCD_SetTextColor(LightGrey);
        GLCD_DisplayChar(8, 10, __FI, 0x80+ 0);
      }
#endif // __USE_LCD

      joy_ = joy;
    }

    /* Printf message with AD value to serial port every 1 second */
    if (clock_1s) {
      clock_1s = 0;
      sprintf(text, "AD value: 0x%04X", AD_print);

      printf("%s\r\n", text);
    }

  }
}
