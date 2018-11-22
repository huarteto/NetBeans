/*----------------------------------------------------------------------------
 * Name:    Blinky.c
 * Purpose: LED Flasher for EK-LM3S811
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
#include "lm3s_cmsis.h"

#define LED_NUM     1                   /* Number of user LEDs                */
const unsigned long led_mask[] = { 1UL<< 0 };

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
  uint32_t i;

  SYSCTL->RCGC0 |= (1<<16);               /* Enable power to ADC block          */
  for (i = 0; i < 0x10; i++) __NOP();     /* wait */

  ADC->ACTSS     = (1<< 0);               /* ADC SS0 Enable                     */
  ADC->EMUX      = 0;                     /* SS0 trigger select is controller   */

//  ADC->SSCTL0    = (1<<30) |              /* generate interrupt after 8th sample*/
//                   (1<<29);               /* 8th Sample is End of Sequence      */
  ADC->SSCTL0    = (1<< 2) |              /* generate interrupt after 1st sample*/
                   (1<< 1);               /* 1st Sample is End of Sequence      */
  ADC->IM        = (1<< 0);               /* enable ADC SS0 Interrupt           */

  NVIC_EnableIRQ(ADCSeq0_IRQn);           /* enable ADC SS0 Interrupt           */

}


/*----------------------------------------------------------------------------
  Function that initializes LEDs
 *----------------------------------------------------------------------------*/
void LED_init(void) {
  uint32_t i;

  SYSCTL->RCGC2 |= (1<<5);                /* enable clock for GPIOF */
  for (i = 0; i < 0x10; i++) __NOP();     /* wait */
  GPIOF->DIR     = 0x01;                  /* Enable the GPIO pin for output */
  GPIOF->DEN     = 0x01;                  /* enable the GPIO pin */
}

/*----------------------------------------------------------------------------
  Function that turns on requested LED
 *----------------------------------------------------------------------------*/
void LED_On (unsigned int num) {

  GPIOF->DATA |= led_mask[num];
}

/*----------------------------------------------------------------------------
  Function that turns off requested LED
 *----------------------------------------------------------------------------*/
void LED_Off (unsigned int num) {

    GPIOF->DATA &= ~led_mask[num];
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
