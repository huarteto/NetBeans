/******************************************************************************/
/* Blinky.c: LED Flasher                                                      */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2005-2009 Keil Software. All rights reserved.                */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/
                  
#include <AT91SAM9G20.H>                /* AT91SAM9G20 definitions            */

#define CLOCK_FREQ  99072000            /* Master clock frequency in Hz       */
#define LED_NUM     1                   /* Number of user LEDs                */
const int led_mask[] = { AT91C_PIO_PA6 };

volatile unsigned long SysTickCnt;


__irq void PIT_Handler (void)  {        /* PIT Interrupt Handler (1 ms)       */
  SysTickCnt++;
  *AT91C_AIC_EOICR = AT91C_BASE_PITC->PITC_PIVR;
}

void Delay (unsigned long tick) {       /* Delay Function                     */
  unsigned long systickcnt;

  systickcnt = SysTickCnt;
  while ((SysTickCnt - systickcnt) < tick);
}

int main (void) {                       /* Main Program                       */
  int num = -1; 
  int dir =  1;

  AT91C_BASE_PMC->PMC_PCER  = 1 << AT91C_ID_PIOA;     /* Enable clock for LED */
  AT91C_BASE_PIOA->PIO_OER  = led_mask[0];            /* Config LED as output */
  AT91C_BASE_PIOA->PIO_SODR = led_mask[0];            /* Clear the LED        */

  /* Setup System Timer to generate interrupt every 1 ms                      */
  AT91C_BASE_SYS->PITC_PIMR  = CLOCK_FREQ/16/1000 - 1;
  AT91C_BASE_SYS->PITC_PIMR |= AT91C_PITC_PITIEN;
  AT91C_BASE_AIC->AIC_SVR[AT91C_ID_SYS] = (unsigned int) PIT_Handler;
  AT91C_BASE_AIC->AIC_SMR[AT91C_ID_SYS] = AT91C_AIC_SRCTYPE_INT_LEVEL_SENSITIVE;
  AT91C_BASE_AIC->AIC_IECR   = (1<<AT91C_ID_SYS);     /* Enable ST interrupt  */
  AT91C_BASE_SYS->PITC_PIMR |= AT91C_PITC_PITEN;      /* Run System Timer     */

  for (;;) {                            /* Loop forever                       */
    /* Calculate 'num': 0,1,...,LED_NUM-1,LED_NUM-1,...,1,0,0,...             */
    num += dir;
    if (num == LED_NUM) { dir = -1; num =  LED_NUM-1; } 
    else if   (num < 0) { dir =  1; num =  0;         }

    AT91C_BASE_PIOA->PIO_CODR = led_mask[num];        /* Turn LED on          */
    Delay(500);                                       /* Delay 500ms          */
    AT91C_BASE_PIOA->PIO_SODR = led_mask[num];        /* Turn LED off         */
    Delay(500);                                       /* Delay 500ms          */
  }
}
