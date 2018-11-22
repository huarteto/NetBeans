/*----------------------------------------------------------------------------
 * Name:    Blinky.c
 * Purpose: LED Flasher for MCBTMPM330
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
#include "TMPM330.H"                    /* TMPM330 definitions                */

#define LED_NUM     3                   /* Number of user LEDs                */
const unsigned long led_mask[] = { 1UL<< 0,  1UL<< 1, 1UL<< 2 };

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

  TSB_AD->MOD1 = (4 << 0) |             /* Input Channel: AIN4                */
             (1 << 7) ;                 /* VREF=ON                            */

  TSB_AD->CLK  = (9 << 4) |             /* hold time = 16 conversion clocks   */
             (4 << 0);                  /* A/D prescaler = fc/16              */
  TSB_AD->MOD0 = (2 << 3);              /* generate interrupt once every 8 conversions */

  NVIC_EnableIRQ(INTAD_IRQn);           /* enable ADC Interrupt               */
}


/*----------------------------------------------------------------------------
  Function that initializes LEDs
 *----------------------------------------------------------------------------*/
void LED_init(void) {

  TSB_PI->CR   |=  0x07;                /* Enable LED pins as outputs         */
  TSB_PI->DATA |=  0x07;                /* Turn all LEDs off                  */
}

/*----------------------------------------------------------------------------
  Function that turns on requested LED
 *----------------------------------------------------------------------------*/
void LED_On (unsigned int num) {

  TSB_PI->DATA &= ~led_mask[num];       /* Turn LED on                        */
}

/*----------------------------------------------------------------------------
  Function that turns off requested LED
 *----------------------------------------------------------------------------*/
void LED_Off (unsigned int num) {

  TSB_PI->DATA |=  led_mask[num];       /* Turn LED off                       */
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

  SysTick_Config(SystemCoreClock/100);  /* Generate interrupt each 10 ms      */

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
