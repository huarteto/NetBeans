/******************************************************************************/
/* BLINKY.C: LED Flasher                                                      */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2005-2009 Keil Software. All rights reserved.                */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/
                  
#include <AT91SAM9261.H>                /* AT91SAM9261 definitions            */
#include "GLCD.h"

#define CLOCK_FREQ  96109714            /* Main clock frequency in Hz         */

#define LED_NUM     2                   /* Number of user LEDs                */

const int led_mask[] = { AT91C_PIO_PA14, AT91C_PIO_PA13 }; /* LEDs DS7 and DS8*/

volatile unsigned char Clock100ms;
volatile unsigned char Clock1s;

volatile char TimeFlag;
volatile long TimeTick;

extern unsigned char Background_32bpp_rl_fv[];
extern unsigned char ARM_Ani_32bpp_rl_fv[];


/*
 * Period Interrupt Timer (PIT) - interrupt function (every ~10 ms)
 * Activate Clock100ms (every ~10ms * 10) and Clock1s (every ~10ms * 100)
 */  

__irq void PIT_Handler (void)  {
  volatile static unsigned long ticks;

  if (!((ticks++ ^ 9) & 0x0F)) {        /* Set Clock100ms to 1 every 100 ms   */
    ticks      &= ~0x0F;
    ticks      +=  0x10;
    Clock100ms  =  1;
  }
  if (ticks >= (10 << 4)) {             /* Set Clock1s to 1 every 1 second    */
    ticks   = 0;
    Clock1s = 1;
  }

  *AT91C_AIC_EOICR = AT91C_BASE_PITC->PITC_PIVR;
}


/*
 * Activate LED with index, state 0 = OFF, 1 = ON
 */  

void LED (unsigned int index, unsigned int state)  {
  if (state) AT91C_BASE_PIOA->PIO_CODR = led_mask[index];
  else       AT91C_BASE_PIOA->PIO_SODR = led_mask[index];
}


int main (void) {

  int num     = -1; 
  int dir     =  1;
  int off     =  0; 
  int pic     =  0;
  int clk_ani =  0;

  /* Enable the Clock of the PIO for LEDs                                     */
  AT91C_BASE_PMC->PMC_PCER  = 1 << AT91C_ID_PIOA;

  /* Configure the PIO Lines corresponding to LEDs DS7 and DS8 as Output      */
  AT91C_BASE_PIOA->PIO_OER  = led_mask[0] | led_mask[1];

  /* Clear the LEDs. On the Board we must apply a "1" to turn off LEDs        */
  AT91C_BASE_PIOA->PIO_SODR = led_mask[0] |led_mask[1];

  /* Periodic Interval Timer initialization (generate interrupt every ~ 10 ms)*/
  AT91C_BASE_SYS->PITC_PIMR  = CLOCK_FREQ/16/100;
  AT91C_BASE_SYS->PITC_PIMR |= AT91C_PITC_PITIEN;
  AT91C_BASE_AIC->AIC_SVR[AT91C_ID_SYS] = (unsigned int) PIT_Handler;
  AT91C_BASE_AIC->AIC_SMR[AT91C_ID_SYS] = AT91C_AIC_SRCTYPE_INT_HIGH_LEVEL;

  /* Enable Periodic Interval Timer (PIT) interrupt                           */
  AT91C_BASE_AIC->AIC_IECR = (1<<AT91C_ID_SYS);

  /* Run Periodic Interval Timer                                              */
  AT91C_BASE_SYS->PITC_PIMR |= AT91C_PITC_PITEN;

  GLCD_init();
  GLCD_bitmap (0, 0, 240, 320, Background_32bpp_rl_fv);

  for (;;) {                            /* Loop forever                       */

    if (Clock100ms) {
      Clock100ms = 0;

      if (clk_ani++ == 1) {             /* Draw animation picture every 200 ms*/
        clk_ani = 0;
        if (pic++ > 8) pic = 0;
        GLCD_bitmap (99, 99, 45, 120, &ARM_Ani_32bpp_rl_fv[pic*(45*120*4)]);
      }
    }

    if (Clock1s) {                      /* Blink LED every 1 second           */
      Clock1s = 0;

      /* Calculate 'num': 0, 1, ... , LED_NUM-1, LED_NUM-1, ... , 1, 0, 0, ...*/
      num += dir;
      if (num == LED_NUM) { dir = -1; num =  LED_NUM-1; } 
      else if   (num < 0) { dir =  1; num =  0;         }

      if (!off)
        LED (num, 1);                   /* Turn  on LED with index 'num'      */
      else 
        LED (num, 0);                   /* Turn off LED with index 'num'      */

      off = !off;
    }
  }
}
