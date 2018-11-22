/*----------------------------------------------------------------------------
 * Name:    Blinky.c
 * Purpose: LED Flasher for MCBSTM32E
 * Note(s):
 *----------------------------------------------------------------------------
 * This file is part of the uVision/ARM development tools.
 * This software may only be used under the terms of a valid, current,
 * end user licence from KEIL for a compatible version of KEIL software
 * development tools. Nothing else gives you the right to use this software.
 *
 * This software is supplied "AS IS" without warranties of any kind.
 *
 * Copyright (c) 2009 Keil - An ARM Company. All rights reserved.
 *----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stm32f10x_cl.h>

#define LED_NUM     8                   /* Number of user LEDs                */
const unsigned long led_mask[] = { 1UL<<15, 1UL<<14, 1UL<<13, 1UL<<12,
                                   1UL<<11, 1UL<<10, 1UL<< 9, 1UL<< 8 };

/* Import external functions from Serial.c file                               */
extern void SER_init (void);

/* Import external variables from IRQ.c file                                  */
extern volatile unsigned short AD_last;
extern volatile unsigned char  clock_1s;

/* variable to trace in LogicAnalyzer (should not read to often)              */
       volatile unsigned short AD_dbg;          

/*----------------------------------------------------------------------------
  Function that initializes ADC
 *----------------------------------------------------------------------------*/
void ADC_init (void) {

  RCC->APB2ENR |= (1<<4);               /* enable periperal clock for GPIOC   */
  GPIOC->CRL &= ~0x000F0000;            /* Configure PC4 as ADC.14 input      */

  RCC->APB2ENR |= (1<<9);               /* enable periperal clock for ADC1    */

  ADC1->SQR1    = 0x00000000;           /* Regular channel 1 conversion       */
  ADC1->SQR2    = 0x00000000;           /* Clear register                     */
  ADC1->SQR3    = (14<< 0);             /* SQ1 = channel 14                   */
  ADC1->SMPR1   = ( 5<<12);             /* sample time channel 14 55,5 cycles */
  ADC1->CR1     = (1 <<  8) |           /* Scan mode on                       */
                  (1 <<  5) ;           /* EOC interrupt enable               */
  ADC1->CR2     = (1 << 20) |           /* Enable external trigger            */
                  (7 << 17) |           /* EXTSEL = SWSTART                   */
                  (1 <<  0) ;           /* ADC enable                         */
  ADC1->CR2    |= (1 <<  3);            /* Reset calibration                  */
  while (ADC1->CR2 & (1 << 3));         /* wait unil reset finished           */

  ADC1->CR2    |= (1 <<  2);            /* start calibration                  */
  while (ADC1->CR2 & (1 << 2));         /* wait unil calibration finished     */

  NVIC_EnableIRQ(ADC_IRQn);             /* enable ADC Interrupt               */
}


/*----------------------------------------------------------------------------
  Function that initializes LEDs
 *----------------------------------------------------------------------------*/
void LED_init(void) {

  RCC->APB2ENR |= (1<<6);               /* Enable GPIOE clock                 */
  GPIOE->CRH    = 0x33333333;           /* Configure the GPIO for LEDs        */
}

/*----------------------------------------------------------------------------
  Function that turns on requested LED
 *----------------------------------------------------------------------------*/
void LED_On (unsigned int num) {

  GPIOE->BSRR = led_mask[num];
}

/*----------------------------------------------------------------------------
  Function that turns off requested LED
 *----------------------------------------------------------------------------*/
void LED_Off (unsigned int num) {

  GPIOE->BRR = led_mask[num];
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
  MAIN function
 *----------------------------------------------------------------------------*/
int main (void) {                       /* Main Program                       */
  short AD_value, AD_print;

  SystemInit();
  SysTick_Config(SystemFrequency/100);  /* Generate interrupt each 10 ms      */

  LED_init();                           /* LED Initialization                 */
  SER_init();                           /* UART#1 Initialization              */
  ADC_init();                           /* ADC Initialization                 */

  while (1) {                           /* Loop forever                       */
    if (AD_value != AD_last) {          /* Make sure that AD interrupt did    */
      AD_value = AD_last;               /* not interfere with value reading   */
      AD_dbg   = AD_value;
    }
    AD_print  = AD_value;               /* Get unscaled value for printout    */

    if (clock_1s) {
      clock_1s = 0;
      printf ("AD value = 0x%03x\n\r", AD_print);
    }
  }
}
