/******************************************************************************/
/* BLINKY.C: LED Flasher                                                      */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2005-2009 Keil Software. All rights reserved.                */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/
                  
#include <AT91SAM9RL64.H>               /* AT91SAM9RL64 definitions           */

#define MCK_FREQ    100000000           /* Main clock frequency in Hz         */
#define LED_NUM     2                   /* Number of user LEDs                */

const int led_mask[] = { 1 << 15, 1 << 16 };

volatile char TimeFlag;
volatile long TimeTick;


void wait (void) {                      /* Wait until TimeFlag is activated   */
  while (!TimeFlag);
  TimeFlag = 0;
}

__irq void PIT_Handler (void)  {        /* Period Interrupt Timer (PIT) (10ms)*/
  if (TimeTick ++ >= 99) {
    TimeFlag = 1;
    TimeTick = 0;
  }
  *AT91C_AIC_EOICR = AT91C_BASE_PITC->PITC_PIVR;
}

int main (void) {
  int num = -1; 
  int dir =  1;

  /* Enable the Clock of the PIO for LEDs                                     */
  AT91C_BASE_PMC->PMC_PCER      |= 1 << AT91C_ID_PIOD;

  /* Configure the PIO Lines corresponding to LEDS DS1 and DS2 as Outputs     */
  AT91C_BASE_PIOD->PIO_OER      |= led_mask[0] | led_mask[1];

  /* Clear the LED's. On the Board we must apply a "1" to turn off LEDs       */
  AT91C_BASE_PIOD->PIO_SODR     |= led_mask[0] | led_mask[1];

  /* Setup Periodic Interval Timer to generate interrupt every 10ms           */
  AT91C_BASE_SYS->SYS_PITC_PIMR  = (MCK_FREQ/16/100)-1;
  AT91C_BASE_SYS->SYS_PITC_PIMR |= AT91C_PITC_PITIEN;
  AT91C_BASE_AIC->AIC_SVR[AT91C_ID_SYS] = (unsigned int) PIT_Handler;
  AT91C_BASE_AIC->AIC_SMR[AT91C_ID_SYS] = AT91C_AIC_SRCTYPE_INT_LEVEL_SENSITIVE;

  /* Enable System Interrupt                                                  */
  AT91C_BASE_AIC->AIC_IECR      |= (1<<AT91C_ID_SYS);

  /* Run Periodic Interval Timer                                              */
  AT91C_BASE_SYS->SYS_PITC_PIMR |= AT91C_PITC_PITEN;

  for (;;) {                            /* Loop forever                       */
    /* Calculate 'num': 0,1,...,LED_NUM-1,LED_NUM-1,...,1,0,0,...             */
    num += dir;
    if (num == LED_NUM) { dir = -1; num =  LED_NUM-1; } 
    else if   (num < 0) { dir =  1; num =  0;         }

    AT91C_BASE_PIOD->PIO_CODR = led_mask[num];
    wait();
    AT91C_BASE_PIOD->PIO_SODR = led_mask[num];
    wait();
  }
}
