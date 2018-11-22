/******************************************************************************/
/* Blinky.c: LED Flasher                                                      */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2010 Keil Software. All rights reserved.                     */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/
                  
#include <stdio.h>
#include <MB9BF50x.h>                   /* MB9BF50x definitions               */

#define LED_NUM     8                   /* Number of user LEDs                */
const unsigned long led_mask[] = { 1UL <<  8, 1UL <<  9, 1UL << 10, 1UL << 11,
                                   1UL << 12, 1UL << 13, 1UL << 14, 1UL << 15};

               char text[40];

/* Import external function from Serial.c file                                */
extern void SER_init (void);

/* Import external variables from IRQ.c file                                  */
extern unsigned short AD_last;
extern unsigned char  clock_1s;

/* variable to trace in LogicAnalyzer (should not read to often)              */
       volatile unsigned short AD_dbg;          

/*----------------------------------------------------------------------------
  Function that initializes ADC
 *----------------------------------------------------------------------------*/
void ADC_init (void) {

  MB9BF_GPIO->PFR[1]  &= ~(0x0001);     /* P10 is GPIO                        */
  MB9BF_GPIO->DDR[1]  &= ~(0x0001);     /* P10 is input                       */
  MB9BF_GPIO->ADE     |=  (0x0001);     /* P10 is analog input                */

  MB9BF_ADC0->ADCR     = ((1 << 3) |    /* enable Scan conversion interrupt   */
                          (0 << 0) );
  MB9BF_ADC0->SFNS     =  (0x00);       /* only use one FIFO stage            */
  MB9BF_ADC0->SCIS0    =  (0x01);       /* chn. AN0 selected for conversion   */
  MB9BF_ADC0->ADSS0   &= ~(0x01);       /* use ADST0 sampling time register   */
  MB9BF_ADC0->ADST0    =  (0x09);       /* 2,5us @ 40MHz (25ns * (9 + 1) * 1) */

  NVIC_EnableIRQ(ADC0_IRQn);            /* enable ADC Interrupt               */
}


/*----------------------------------------------------------------------------
  Function that initializes LEDs
 *----------------------------------------------------------------------------*/
void LED_init(void) {
  MB9BF_GPIO->PFR[3]  &= ~(0xFF00);     /* LEDs on port 3 defined as GPIO     */
  MB9BF_GPIO->PDOR[3] |=  (0xFF00);     /* switch off LEDs                    */
  MB9BF_GPIO->DDR[3]  |=  (0xFF00);     /* LEDs on port 3 defined as output   */
  MB9BF_GPIO->PCR[3]  &= ~(0xFF00);     /* LEDs on port 3 do not use pull-up  */
}

/*----------------------------------------------------------------------------
  Function that turns on requested LED
 *----------------------------------------------------------------------------*/
void LED_On (unsigned int num) {

  MB9BF_GPIO->PDOR[3] &= ~led_mask[num];
}

/*----------------------------------------------------------------------------
  Function that turns off requested LED
 *----------------------------------------------------------------------------*/
void LED_Off (unsigned int num) {

  MB9BF_GPIO->PDOR[3] |=  led_mask[num];
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
  unsigned short AD_value, AD_print = 0;

  SysTick_Config(SystemCoreClock/100);  /* Generate interrupt each 10 ms      */

  LED_init();                           /* LED Initialization                 */
  ADC_init();                           /* ADC Initialization                 */
  SER_init();                           /* UART Initialization                */

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
      printf ("AD value = 0x%04X\n\r", AD_print);
    }

  }
}
