/******************************************************************************/
/* BLINKY.C: LED Flasher                                                      */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2005-2007 Keil Software. All rights reserved.                */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/
                  
#include <AT91SAM9263.H>                /* AT91SAM9263 definitions            */

#define CLOCK_FREQ  100024589           /* Main clock frequency in Hz         */

#define LED_NUM     2                   /* Number of user LEDs                */

const int led_mask[] = { AT91C_PIO_PB8, AT91C_PIO_PC29 };  /* LEDs DS1 and DS2*/

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
 * Activate LED with index, state 0 = OFF, 1 = ON
 */  

void LED (unsigned int index, unsigned int state)  {
  switch (index) {
    case 0:
      if (state) AT91C_BASE_PIOB->PIO_CODR = led_mask[0];
      else       AT91C_BASE_PIOB->PIO_SODR = led_mask[0];
      break;
    case 1:
      if (state) AT91C_BASE_PIOC->PIO_CODR = led_mask[1];
      else       AT91C_BASE_PIOC->PIO_SODR = led_mask[1];
      break;
  }
}


/*
 * Main Program
 */

int main (void) {
  int i;

  // Enable the Clock of the PIO for LEDs
  AT91C_BASE_PMC->PMC_PCER  = 1 << AT91C_ID_PIOB;
  AT91C_BASE_PMC->PMC_PCER  = 1 << AT91C_ID_PIOCDE;

  // Configure the PIO Lines corresponding to LEDs DS1 and DS2 as Output
  AT91C_BASE_PIOB->PIO_OER  = led_mask[0];
  AT91C_BASE_PIOC->PIO_OER  = led_mask[1];

  // Clear the LEDs. On the Board we must apply a "1" to turn off LEDs
  AT91C_BASE_PIOB->PIO_SODR = led_mask[0];
  AT91C_BASE_PIOC->PIO_SODR = led_mask[1];

  // System Timer initialization
  AT91C_BASE_SYS->SYS_PITC_PIMR  = CLOCK_FREQ/16/100; // Interrupt time interval ~10 ms
  AT91C_BASE_SYS->SYS_PITC_PIMR |= AT91C_PITC_PITIEN;
  AT91C_BASE_AIC->AIC_SVR[AT91C_ID_SYS] = (unsigned int) PIT_Handler;
  AT91C_BASE_AIC->AIC_SMR[AT91C_ID_SYS] = AT91C_AIC_SRCTYPE_INT_HIGH_LEVEL;

  // Enable ST interrupt
  AT91C_BASE_AIC->AIC_IECR = (1<<AT91C_ID_SYS);

  // Run System Timer
  AT91C_BASE_SYS->SYS_PITC_PIMR |= AT91C_PITC_PITEN;

  // Loop forever
  for (;;) {
    for (i = 0; i < LED_NUM; i++) {
      LED (i, 1);
      wait();
      LED (i, 0);
      wait();
    }
    for (i = (LED_NUM - 1); i >= 0; i--) {
      LED (i, 1);
      wait();
      LED (i, 0);
      wait();
    }
  }
}
