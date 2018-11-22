/******************************************************************************/
/* BLINKY.C: LED Flasher                                                      */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2005-2006 Keil Software. All rights reserved.                */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/
                  
#include <AT91RM9200.H>                 /* AT91RM9200 definitions             */

#define LED_NUM               3         /* Number of LEDs                     */

const int led_mask[] = { AT91C_PIO_PB0, AT91C_PIO_PB1, AT91C_PIO_PB2 };

volatile unsigned long TimeTick;


/*
 *   Waiting for defined time (~10 * 10ms)
 */
 
void wait (void) {
  volatile unsigned long tick;
  
  tick = TimeTick;
  while ((TimeTick-tick)<10);
}


/*
 * Period Interrupt Timer (PIT) - interrupt function (every ~10 ms)
 */  

__irq void PIT_Handler (void)  {
  TimeTick++;
  *AT91C_AIC_EOICR = *AT91C_ST_SR;
}


/*
 * Main Program
 */

int main (void) {
  int i;

  // Enable the Clock of the PIO for LEDs
  AT91C_BASE_PMC->PMC_PCER = 1 << AT91C_ID_PIOB;

  // Configure the PIO Lines corresponding to USER LED1 .. USER LED3 as Outputs
  AT91C_BASE_PIOB->PIO_OER = AT91C_PIO_PB0 | AT91C_PIO_PB1 | AT91C_PIO_PB2;

  // Clear the LED's. On the Board we must apply a "1" to turn off LEDs
  AT91C_BASE_PIOB->PIO_SODR = AT91C_PIO_PB0 | AT91C_PIO_PB1 | AT91C_PIO_PB2;

  // System Timer initialization
  AT91C_BASE_SYS->ST_PIMR = 32768 / 100;          // Interrupt time interval ~10 ms
  AT91C_BASE_SYS->ST_IER  = AT91C_ST_PITS;
  AT91C_BASE_AIC->AIC_SVR[AT91C_ID_SYS] = (unsigned int) PIT_Handler;
  AT91C_BASE_AIC->AIC_SMR[AT91C_ID_SYS] = AT91C_AIC_SRCTYPE_INT_LEVEL_SENSITIVE;

  // Enable ST interrupt
  AT91C_BASE_AIC->AIC_IECR = (1<<AT91C_ID_SYS);

  // Loop forever
  for (;;) {
    for (i = 0; i < LED_NUM; i++) {
      AT91C_BASE_PIOB->PIO_CODR = led_mask[i];
      wait();
      AT91C_BASE_PIOB->PIO_SODR = led_mask[i];
      wait();
    }
    for (i = (LED_NUM - 1); i >= 0; i--) {
      AT91C_BASE_PIOB->PIO_CODR = led_mask[i];
      wait();
      AT91C_BASE_PIOB->PIO_SODR = led_mask[i];
      wait();
    }
  }
}
