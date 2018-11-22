/*----------------------------------------------------------------------------
 * Name:    Blinky.c
 * Purpose: MCB1700 LED Flasher
 * Note(s):
 *----------------------------------------------------------------------------
 * This file is part of the uVision/ARM development tools.
 * This software may only be used under the terms of a valid, current,
 * end user licence from KEIL for a compatible version of KEIL software
 * development tools. Nothing else gives you the right to use this software.
 *
 * This software is supplied "AS IS" without warranties of any kind.
 *
 * Copyright (c) 2008 Keil - An ARM Company. All rights reserved.
 *----------------------------------------------------------------------------*/
                  
#include <stdio.h>
#include "LPC17xx.H"                    /* LPC17xx definitions                */
#include "Serial.h"

/* Import external variables from IRQ.c file                                  */
extern unsigned short AD_last;
extern unsigned char  clock_1s;

#define LED_NUM     8                   /* Number of user LEDs                */
const unsigned long led_mask[] = { 1UL<<28, 1UL<<29, 1UL<<31, 1UL<< 2,
                                   1UL<< 3, 1UL<< 4, 1UL<< 5, 1UL<< 6 };


/*----------------------------------------------------------------------------
  Function that initializes ADC
 *----------------------------------------------------------------------------*/
void ADC_init (void) {

  LPC_PINCON->PINSEL1 &= ~(3<<18);      /* P0.25 is GPIO                      */
  LPC_PINCON->PINSEL1 |=  (1<<18);      /* P0.25 is AD0.2                     */

  LPC_SC->PCONP       |=  (1<<12);      /* Enable power to ADC block          */

  LPC_ADC->ADCR        =  (1<< 2) |     /* select AD0.2 pin                   */
                          (4<< 8) |     /* ADC clock is 25MHz/5               */
                          (1<<21);      /* enable ADC                         */ 

  LPC_ADC->ADINTEN     =  (1<< 8);      /* global enable interrupt            */

  NVIC_EnableIRQ(ADC_IRQn);             /* enable ADC Interrupt               */
}


/*----------------------------------------------------------------------------
  Function that initializes LEDs
 *----------------------------------------------------------------------------*/
void LED_init(void) {
  LPC_GPIO1->FIODIR = 0xB0000000;       /* LEDs on PORT1 defined as Output    */
  LPC_GPIO2->FIODIR = 0x0000007C;       /* LEDs on PORT2 defined as Output    */
}

/*----------------------------------------------------------------------------
  Function that turns on requested LED
 *----------------------------------------------------------------------------*/
void LED_On (unsigned int num) {

  if (num < 3) LPC_GPIO1->FIOPIN |= led_mask[num];
  else         LPC_GPIO2->FIOPIN |= led_mask[num];
}

/*----------------------------------------------------------------------------
  Function that turns off requested LED
 *----------------------------------------------------------------------------*/
void LED_Off (unsigned int num) {

    if (num < 3) LPC_GPIO1->FIOPIN &= ~led_mask[num];
    else         LPC_GPIO2->FIOPIN &= ~led_mask[num];
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
  short AD_value, AD_print;

  SystemInit();
  SysTick_Config(SystemFrequency/100);  /* Generate interrupt each 10 ms      */

  LED_init();                           /* LED Initialization                 */
  SER_init(1);                          /* UART#1 Initialization              */
  ADC_init();                           /* ADC Initialization                 */

  while (1) {                           /* Loop forever                       */
    AD_value = AD_last;                 /* Read AD_last value                 */
    if (AD_value != AD_last)            /* Make sure that AD interrupt did    */
      AD_value = AD_last;               /* not interfere with value reading   */
    AD_print  = AD_value;               /* Get unscaled value for printout    */

    if (clock_1s) {
      clock_1s = 0;
      printf ("AD value = 0x%03x\n\r", AD_print);
    }
  }
}
