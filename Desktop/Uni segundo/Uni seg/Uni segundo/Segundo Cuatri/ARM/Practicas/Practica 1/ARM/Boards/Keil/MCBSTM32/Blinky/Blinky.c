/*----------------------------------------------------------------------------
 * Name:    Blinky.c
 * Purpose: LED Flasher for STM32
 * Version: V1.00
 *----------------------------------------------------------------------------
 * This file is part of the uVision/ARM development tools.
 * This software may only be used under the terms of a valid, current,
 * end user licence from KEIL for a compatible version of KEIL software
 * development tools. Nothing else gives you the right to use this software.
 *
 * Copyright (c) 2005-2007 Keil Software. All rights reserved.
 *----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stm32f10x_lib.h>                        // STM32F10x Library Definitions
#include "STM32_Init.h"                           // STM32 Initialization
#include "LCD.h"                                  // LCD function prototypes


unsigned short int  ADC_ConvertedValue;
unsigned       char Clock1s;

/*------------------------------------------------------------------------------
  Initialises the Analog/Digital converter
  PA1 (ADC Channel1) is used as analog input
  use DMA Channel1 for ADC1 (see DMA request mapping)
 *------------------------------------------------------------------------------*/
void adc_Init (void) {

//  GPIOA->CRL &= ~0x0000000F;                    // set PIN1 as analog input (see stm32_Init.c)

  RCC->AHBENR |= (1<<0);                          // enable periperal clock for DMA

  DMA1_Channel1->CMAR  = (u32)&ADC_ConvertedValue;// set channel1 memory address
  DMA1_Channel1->CPAR  = (u32)&(ADC1->DR);        // set channel1 peripheral address
  DMA1_Channel1->CNDTR = 1;                       // transmit 1 word
  DMA1_Channel1->CCR   = 0x00002520;              // configure DMA channel
  DMA1_Channel1->CCR  |= (1 << 0);                // DMA Channel 1 enable


  RCC->APB2ENR |= (1<<9);                         // enable periperal clock for ADC1

  ADC1->SQR1  = 0x00000000;                       // only one conversion
  ADC1->SMPR2 = 0x00000028;                       // set sample time channel1 (55,5 cycles)
  ADC1->SQR3  = 0x00000001;                       // set channel1 as 1st conversion

  ADC1->CR1   = 0x00000100;                       // use independant mode, SCAN mode
  ADC1->CR2   = 0x000E0103;                       // use data align right,continuous conversion
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
    if (leds == 0) leds = 0x01;
    GPIOB->ODR = (GPIOB->ODR & 0xFFFF00FF) | (leds << 8);
  }
} // end SysTick_Handler


/*----------------------------------------------------------------------------
  MAIN function
 *----------------------------------------------------------------------------*/
int main (void) {
  int i;
  int AD_value;
  int AD_scaled_ex = 0;
  int AD_scaled;

  stm32_Init ();                                  // STM32 setup
  adc_Init();                                     // initialise A/D converter

  lcd_init  ();
  lcd_clear ();
  lcd_print (" MCBSTM32 DEMO  ");
  set_cursor(0, 1);
  lcd_print ("  www.keil.com  ");

  for (i = 0; i < 10000000; i++);                 // Wait for initial display

  while (1) {                                     // Loop forever
    AD_value  = ADC_ConvertedValue;               // Read AD value
    AD_scaled = AD_value / 52;                    // AD value scaled to 0 .. 78 (lines on LCD)
    if (AD_scaled != AD_scaled_ex) {              // If new AD value different than old
      AD_scaled_ex = AD_scaled;
      lcd_bargraphXY(0, 1, AD_scaled);            // Display bargraph according to AD
    }

                                                  // Print message with AD value to serial port every second
    if (Clock1s) {
      Clock1s = 0;
      printf("AD value = 0x%04X\r\n", AD_value);
    }
  }
}
