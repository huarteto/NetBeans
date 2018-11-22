/*----------------------------------------------------------------------------
 * Name:    Blinky.c
 * Purpose: LED Flasher for STM32
 * Version: V1.00
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
 *----------------------------------------------------------------------------
 * History:
 *          V1.00 Initial Version
 *----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stm32f10x_lib.h>                        // STM32F10x Library Definitions
#include "STM32_Init.h"                           // STM32 Initialization
#include "LCD.h"                                  // LCD functions

unsigned short int  ADC_ConvertedValue;
unsigned       char Clock1s;

/*------------------------------------------------------------------------------
  Initialises the Analog/Digital converter
  PC0 (ADC Channel10) is used as analog input
  use DMA Channel1 for ADC1 (see DMA request mapping)
 *------------------------------------------------------------------------------*/
void adc_Init (void) {

//  GPIOC->CRL &= ~0x0000000F;                    // set PIN0 as analog input (see stm32_Init.c)

  RCC->AHBENR |= (1<<0);                          // enable periperal clock for DMA

  DMA1_Channel1->CMAR  = (u32)&ADC_ConvertedValue;// set channel1 memory address
  DMA1_Channel1->CPAR  = (u32)&(ADC1->DR);        // set channel1 peripheral address
  DMA1_Channel1->CNDTR = 1;						  // transmit 1 word
  DMA1_Channel1->CCR   = 0x00002520;              // configure DMA channel
  DMA1_Channel1->CCR  |= (1 << 0);                // DMA Channel 1 enable


  RCC->APB2ENR |= (1<<9);                         // enable periperal clock for ADC1

  ADC1->SQR1 &= ~0x00F00000;                      // only one conversion

  ADC1->SMPR1 &= 0x00000007;                      // clear bits 0..2 (channel10)
  ADC1->SMPR1 |= 0x00000005;                      // set sample time (55,5 cycles)

  ADC1->SQR3  &= 0x0000001F;                      // clear bits 0..4
  ADC1->SQR3  |= 0x0000000A;                      // 1. conversion channel 10

  ADC1->CR1   =  0x00000100;                      // use independant mode, SCAN mode
  ADC1->CR2   =  0x000E0103;                      // use data align right,continuous conversion
                                                  // EXTSEL = SWSTART 
                                                  // enable ADC, DMA mode, no external Trigger
  ADC1->CR2  |=  0x00500000;					  // start SW conversion
}

/*------------------------------------------------------------------------------
  Systick Interrupt Handler
  SysTick interrupt happens every 10 ms
 *------------------------------------------------------------------------------*/
void SysTick_Handler (void) {
  static unsigned long ticks;
  static unsigned long timetick;
  static unsigned char leds = 0x01;

  if (ticks++ >= 99) {                           // Set Clock1s to 1 every 1 second
    ticks   = 0;
    Clock1s = 1;
  }
                                                 // Blink the LEDs depending on ADC_ConvertedValue
  if (timetick++ >= (ADC_ConvertedValue>>8)) {
    timetick   = 0;
    leds     <<= 1;
    if (leds > 0x08) leds = 0x01;
    GPIOC->ODR = (GPIOC->ODR & 0xFFFFFF0F) | (leds << 4);
  }
} // end SysTick_Handler

/*----------------------------------------------------------------------------
  MAIN function
 *----------------------------------------------------------------------------*/
int main (void) {
  int i;
  int AD_value;
  char sAd_value[10];

  stm32_Init ();                                  // STM32 setup
  adc_Init();                                     // initialise A/D converter

  lcdWrStr("KEIL");
  for (i = 0; i < 10000000; i++);                 // Wait for initial display

  while (1) {                                     // Loop forever
    AD_value  = ADC_ConvertedValue;               // Read AD value

    if (Clock1s) {                                
      Clock1s = 0;
      printf("AD value = 0x%04X\r\n", AD_value);  // Print AD value to serial port every second

      sprintf(sAd_value, "%4X", AD_value);        // convert value to a string
      lcdWrStr(sAd_value);                        // write value to LCD
    }

  }  // end while
}
