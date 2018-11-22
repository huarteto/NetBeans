/******************************************************************************/
/* BLINKY.C: LED Flasher                                                      */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2010 Keil Software. All rights reserved.                     */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/
                  
#include <stdio.h>
#include <NUC1xx.H>                     /* NUC1xx definitions                 */

#define LED_NUM     8                   /* Number of user LEDs                */
const unsigned long led_mask[] = { 1UL << 0, 1UL << 1, 1UL << 2, 1UL << 3,
                                   1UL << 4, 1UL << 5, 1UL << 6, 1UL << 7};

               char text[40];


/* Import external variables from IRQ.c file                                  */
extern unsigned short AD_last;
extern unsigned char  clock_1s;

/* variable to trace in LogicAnalyzer (should not read to often)              */
       volatile unsigned short AD_dbg;          

/*----------------------------------------------------------------------------
  Function that initializes ADC
 *----------------------------------------------------------------------------*/
void ADC_init (void) {

  NUC_GCR->GPA_MFP |= (1UL << 0);         /* ADC0 function is selected to PA.0*/

  NUC_CLK->APBCLK |= (1UL << 28);         /* A/D Converter Clock Enable       */

  NUC_ADC->CR    = ((2UL << 16) |         /* A/D Converter Clock Divider      */
                    (0UL <<  2) |         /* A/D Converter Single conversion  */ 
                    (1UL <<  1) |         /* A/D Interrupt Enable             */
                    (1UL <<  0)  );       /* A/D Converter Enable             */
  NUC_ADC->CHER  = ((1UL <<  0)  );       /* Analog Input Channel 0 Enable    */

  NVIC_EnableIRQ(ADC_IRQn);               /* enable ADC Interrupt             */
}


/*----------------------------------------------------------------------------
  Function that initializes LEDs
 *----------------------------------------------------------------------------*/
void LED_init(void) {
  NUC_GPIOE->PMD   = 0x00005555;        /* LEDs on PORT E defined as Output   */
  NUC_GPIOE->DOUT |= 0x000000FF;        /* switch off LEDs                    */
}

/*----------------------------------------------------------------------------
  Function that turns on requested LED
 *----------------------------------------------------------------------------*/
void LED_On (unsigned int num) {

  NUC_GPIOE->DOUT  &= ~led_mask[num];
}

/*----------------------------------------------------------------------------
  Function that turns off requested LED
 *----------------------------------------------------------------------------*/
void LED_Off (unsigned int num) {

  NUC_GPIOE->DOUT  |=  led_mask[num];
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
  unsigned short AD_value, AD_print = 0;

  SysTick_Config(SystemCoreClock/100);  /* Generate interrupt each 10 ms      */

  LED_init();                           /* LED Initialization                 */
  ADC_init();                           /* ADC Initialization                 */

  while (1) {                           /* Loop forever                       */

    AD_value = AD_last;                 /* Read AD_last value                 */
    if (AD_value != AD_last)            /* Make sure that AD interrupt did    */
      AD_value = AD_last;               /* not interfere with value reading   */

    if (AD_value != AD_print) {         /* Make sure that AD interrupt did    */
      AD_print = AD_value;              /* Get unscaled value for printout    */
      AD_dbg   = AD_value;
    }

    if (clock_1s) {
      clock_1s = 0;
      sprintf(text, "AD value = 0x%04X", AD_print);
    }

  }
}
