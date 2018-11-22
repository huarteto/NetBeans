/******************************************************************************/
/* BLINKY.C: LED Flasher                                                      */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2005-2009 Keil Software. All rights reserved.                */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/
                  
#include <AT91SAM9XE512.H>              /* AT91SAM9XE definitions             */

#define CLOCK_FREQ  96109714            /* Main clock frequency in Hz         */

#define LED_NUM     1                   /* Number of user LEDs                */

const int led_mask[] = { AT91C_PIO_PA6 };

volatile char TimeFlag;
volatile long TimeTick;


/*
 *   Waiting until TimeFlag is activated
 */
 
void wait (void) {
  while (!TimeFlag);
  TimeFlag = 0;
}


/*
 * Period Interrupt Timer (PIT) - interrupt function (every ~10 ms)
 * Activate TimeFlag (every ~10ms * 100)
 */  

__irq void PIT_Handler (void)  {
  if (TimeTick++ >= 100) {
    TimeFlag = 1;
    TimeTick = 0;
  }
  *AT91C_AIC_EOICR = AT91C_BASE_PITC->PITC_PIVR;
}


/*
 * Main Program
 */

int main (void) {
  int i;

  // Enable the Clock of the PIO for LEDs
  *AT91C_PMC_PCER           = 1 << AT91C_ID_PIOA;

  // Configure the PIO Line corresponding to LED DS5 as Output
  AT91C_BASE_PIOA->PIO_OER  = led_mask[0];

  // Clear the LED. On the Board we must apply a "1" to turn off LED
  AT91C_BASE_PIOA->PIO_SODR = led_mask[0];

  // System Timer initialization
  *AT91C_PITC_PIMR  = CLOCK_FREQ/16/100;     // Interrupt time interval ~10 ms
  *AT91C_PITC_PIMR |= AT91C_PITC_PITIEN;
  AT91C_BASE_AIC->AIC_SVR[AT91C_ID_SYS] = (unsigned int) PIT_Handler;
  AT91C_BASE_AIC->AIC_SMR[AT91C_ID_SYS] = AT91C_AIC_SRCTYPE_INT_LEVEL_SENSITIVE;

  // Enable ST interrupt
  AT91C_BASE_AIC->AIC_IECR = (1<<AT91C_ID_SYS);

  // Run System Timer
  *AT91C_PITC_PIMR |= AT91C_PITC_PITEN;

  // Loop forever
  for (;;) {
    for (i = 0; i < LED_NUM; i++) {
      AT91C_BASE_PIOA->PIO_CODR = led_mask[i];
      wait();
      AT91C_BASE_PIOA->PIO_SODR = led_mask[i];
      wait();
    }
    for (i = (LED_NUM - 1); i >= 0; i--) {
      AT91C_BASE_PIOA->PIO_CODR = led_mask[i];
      wait();
      AT91C_BASE_PIOA->PIO_SODR = led_mask[i];
      wait();
    }
  }
}
