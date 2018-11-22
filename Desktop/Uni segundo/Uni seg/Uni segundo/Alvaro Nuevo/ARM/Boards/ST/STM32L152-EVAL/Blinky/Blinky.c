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
#include <stm32l1xx.h>                  /* STM32L1xx.h definitions            */
#include "LCD.h"
#include "GLCD.h"

#define LED_NUM     3                   /* Number of user LEDs                */
const unsigned long led_mask[] = { 1UL <<  0, 1UL <<  1, 1UL << 4};

               char text[40];

/* Import external function from Serial.c file                                */
extern void SER_init (void);

/* Import external variables from IRQ.c file                                  */
extern unsigned short AD_last;
extern unsigned char  clock_1s;
extern unsigned char  clock_50ms;       /* Flag activated each 50  ms         */

unsigned char Demo_text[] = "* KEIL *";
unsigned char LCD_text[]  = "        ";

/* variable to trace in LogicAnalyzer (should not read to often)              */
       volatile unsigned short AD_dbg;          


/*------------------------------------------------------------------------------
  note: 
  LCD corrupts AD values due to HW design.
  set __USE_LCD in "options for target - C/C++ - Define" to enable Output on LCD
  set __USE_IRQ in "options for target - C/C++ - Define" to enable ADC in IRQ mode
                                                        default is ADC in DMA mode
 *------------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------
  Function that initializes RCC clock (used for LCD)
 *----------------------------------------------------------------------------*/
void RCC_Init(void) {

  RCC->APB1ENR  |= (1UL << 28) ;        /* enable PWR clock                   */

  PWR->CR  |=  (1UL <<  8);             /* enable access to RCC_CSR           */

  RCC->CSR |=  (1UL << 23);             /* start reset RTC                    */
  RCC->CSR &= ~(1UL << 23);             /* end   reset RTC                    */

  RCC->CSR |= (1UL << 8);               /* enable LSE                         */
  while (!(RCC->CSR & (1UL << 9)));     /* wait until LSE ready               */ 

  RCC->CSR |=  (1UL << 16);             /* LSE clock used as RTC clock        */
}


/*----------------------------------------------------------------------------
  Function that initializes ADC
 *----------------------------------------------------------------------------*/
void ADC_init (void) {
  RCC->CR      |= ( 1UL <<    0);       /* Enable Internal high-speed clock   */
  RCC->APB2ENR |= ( 1UL <<    9);       /* Enable ADC1 clock                  */
  RCC->AHBENR  |= ( 1UL <<    1);       /* Enable GPIOB clock                 */
  GPIOB->MODER |= ( 3UL << 2*12);       /* PB.12 is Analog mode               */

#ifndef __USE_IRQ
  /* DMA1 Channel1 configuration ---------------------------------------------*/
  RCC->AHBENR  |= (1UL << 24);          /* Enable DMA1 clock                  */

  DMA1_Channel1->CMAR  = (uint32_t)&AD_last;    /* Set memory     address     */
  DMA1_Channel1->CPAR  = (uint32_t)&(ADC1->DR); /* Set peripheral address     */
  DMA1_Channel1->CNDTR = 1;             /* Transmit 1 data item               */
  DMA1_Channel1->CCR   = 0x00000520;    /* configure DMA stream               */
  DMA1_Channel1->CCR  |= (1UL << 0);    /* Enable stream                      */
#endif //__USE_IRQ

  /* Setup and initialize ADC converter                                       */
  ADC1->SQR1    = 0x00000000;           /* Regular channel, 1 conversion      */
  ADC1->SQR2    = 0x00000000;           /* Clear register                     */
  ADC1->SQR3    = 0x00000000;           /* Clear register                     */
  ADC1->SQR4    = 0x00000000;           /* Clear register                     */
  ADC1->SQR5    = (18UL <<  0);         /* SQ1 = channel 18                   */
  ADC1->SMPR1   = 0x00000000;           /* Clear register                     */
  ADC1->SMPR2   = ( 4UL << 24);         /* Channel 18 sample time is 48 cyc   */
  ADC1->SMPR3   = 0x00000000;           /* Clear register                     */
  ADC1->CR1     = ( 1UL <<  8);         /* Scan mode on                       */

#ifndef __USE_IRQ
  ADC1->CR2   |=  ( 1UL <<  8);         /* DMA mode enable                    */
  ADC1->CR2   |=  ( 1UL <<  9);         /* continuous DMA requests            */
#else
  ADC1->CR1    |= ( 1UL <<  5);         /* enable EOC interrupt               */
  NVIC_EnableIRQ(ADC1_IRQn);            /* enable ADC Interrupt               */
#endif //__USE_IRQ

  ADC1->CR2    |= ( 1UL <<  0);         /* ADC enable                         */
}


/*----------------------------------------------------------------------------
  Function that initializes LEDs
 *----------------------------------------------------------------------------*/
void LED_init(void) {
  RCC->AHBENR  |= (1 << 3);             /* Enable GPIOD clock                 */

  GPIOD->ODR   |=  0x00000013;          /* switch off LEDs                    */
  GPIOD->MODER &= ~0x0000030F;
  GPIOD->MODER |=  0x00000105;          /* General purpose output mode        */
}

/*----------------------------------------------------------------------------
  Function that turns on requested LED
 *----------------------------------------------------------------------------*/
void LED_On (unsigned int num) {

  GPIOD->BSRRH = led_mask[num];
}

/*----------------------------------------------------------------------------
  Function that turns off requested LED
 *----------------------------------------------------------------------------*/
void LED_Off (unsigned int num) {

  GPIOD->BSRRL = led_mask[num];
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
  unsigned short LCD_cnt, LCD_pos = 0;
  unsigned short AD_value, AD_print = 0;

  SysTick_Config(SystemCoreClock/100);  /* Generate interrupt each 10 ms      */

  RCC_Init();
  LED_init();                           /* LED Initialization                 */
  SER_init();                           /* UART Initialization                */
  ADC_init();                           /* ADC Initialization                 */
  LCD_Init();                           /* LCD Initialization                 */

#ifdef __USE_LCD
  GLCD_Init();                          /* Initialize graphical LCD display   */

  GLCD_Clear(White);                    /* Clear graphical LCD display        */
  GLCD_SetBackColor(Blue);
  GLCD_SetTextColor(White);
  GLCD_DisplayString(0, 0, "STM32L152-EVAL Demo ");
  GLCD_DisplayString(1, 0, "       Blinky       ");
  GLCD_DisplayString(2, 0, "    www.keil.com    ");
  GLCD_SetBackColor(White);
  GLCD_SetTextColor(Blue);
#endif // __USE_LCD

  while (1) {                           /* Loop forever                       */

    AD_value = AD_last;                 /* Read AD_last value                 */
    if (AD_value != AD_last)            /* Make sure that AD interrupt did    */
      AD_value = AD_last;               /* not interfere with value reading   */

    if (AD_value != AD_print) {        /* Make sure that AD interrupt did    */
#ifdef __USE_LCD
      GLCD_SetTextColor(Red);
      GLCD_Bargraph (144, Line6, 160, 20, (AD_value>>2));
      GLCD_SetTextColor(White);
#endif // __USE_LCD

      AD_print = AD_value;              /* Get unscaled value for printout    */
      AD_dbg   = AD_value;
    }

    /* update LCD every 50 milli seconds */
    if (clock_50ms) {
      clock_50ms = 0;

      for (LCD_cnt = 0; LCD_cnt < 8; LCD_cnt++)
        LCD_text[((LCD_pos + LCD_cnt) & 0x07)] =  Demo_text[LCD_cnt];
      LCD_DisplayString(0, LCD_text);

      LCD_pos++;
      if (LCD_pos > 7) LCD_pos = 0;
    }

    /* Printf message with AD value to serial port every 1 second */
    if (clock_1s) {
      clock_1s = 0;
      sprintf(text, "AD value = 0x%04X", AD_print);

#ifdef __USE_LCD
      GLCD_SetTextColor(Blue);
      GLCD_DisplayString(5, 0,  (unsigned char *)text);
#endif // __USE_LCD
      printf("%s\r\n", text);
    }

  }
}
