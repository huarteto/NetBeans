/******************************************************************************/
/* Blinky.c: LED Flasher                                                      */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2007-2009 Keil Software. All rights reserved.                */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/
                  
#include <stdio.h>
#include <LPC313x.H>

#define LED_NUM     8                   /* Number of user LEDs                */

extern volatile unsigned char Clock;

void LED_Init (void) {                  /* Function that initializes LEDs     */

  CGU_SB->clk_pcr[CGU_SB_I2C0_PCLK_ID] |= 1;      /* Enable power to I2C0     */
  CGU_SB->clk_esr[CGU_SB_I2C0_PCLK_ID] |= 1;      /* Enable I2C0              */

  I2C0->ckh = 60;
  I2C0->ckl = 60;
}

void LED_Out (int led, int on) {        /* Function that outputs value to LEDs*/
  unsigned int val = (1 << (led << 1)) * (on == 1);

  I2C0->fifo = I2C_TXFF_START_CND |  0xC0;
  I2C0->fifo =                       0x18;
  I2C0->fifo =                      ( val        & I2C_TXFF_DATA_MSK);
  I2C0->fifo = I2C_TXFF_STOP_CND  | ((val >> 8)  & I2C_TXFF_DATA_MSK);
  while (!(I2C0->sts & I2C_STS_TDI));   /* Wait for STOP                      */
}


int main (void) {
  int num = -1; 
  int dir =  1;
  int off =  1; 

  LED_Init();                           /* LED Initialization                 */

  /* Setup Timer 3 to generate interrupt every 10 ms                          */
  CGU_SB->clk_pcr[CGU_SB_TIMER3_PCLK_ID] |= 1;    /* Enable power to Timer3   */
  TIMER_CNTR3->load         = 6000000/100;
  TIMER_CNTR3->control      = (1 << 7) | (1 << 6);

  /* Setup and enable Timer 3 interrupt                                       */
  INTC->irq_vec             = 0;
  INTC->irq_prio_mask       = 0;
  INTC->request[IRQ_TIMER3] = (1 << 28) | (1 << 26) | (1 << 16) | 1;

  for (;;) {                            /* Loop forever                       */
    if (Clock) {                        /* Blink LED every 1 second           */
      Clock = 0;
      if (!off) {
        /* Calculate 'num': 0, 1, ... , LED_NUM-1, LED_NUM-1, ... ,1,0,0, ... */
        num += dir;
        if (num == LED_NUM) { dir = -1; num =  LED_NUM-1; } 
        else if   (num < 0) { dir =  1; num =  0;         }

        LED_Out(num, 1);                /* Turn  on LED with index 'num'      */
      } else {
        LED_Out(num, 0);                /* Turn off LED with index 'num'      */
      }
      off = !off;
    }
  }
}
