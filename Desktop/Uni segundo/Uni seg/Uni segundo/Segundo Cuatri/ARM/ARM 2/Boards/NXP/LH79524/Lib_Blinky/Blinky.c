/******************************************************************************/
/* BLINKY.C: LED Flasher                                                      */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2005-2007 Keil Software. All rights reserved.                */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/
                  
#include <LH79524.H>                    /* NXP LH79524 and LH79525 registers  */
#include <LH79524_LIB.H>                /* NXP LH79524 and LH79525 lib funcs  */

#define LED_NUM     2                   /* Number of user LEDs                */
#define LED0        (1 << 1)            /* LED STATUS0                        */
#define LED1        (1 << 2)            /* LED STATUS1                        */
#define LED_MSK     (LED0 | LED1)       /* Mask of all LEDs                   */

const int led_mask[] = {LED0, LED1};    /* Status LEDs                        */

extern volatile unsigned char Clock;

extern __irq void TIMER0_IRQ_Handler (void);


void wait (void) {                      /* Wait Function                      */

  while (!Clock);
  Clock = 0;
}

static unsigned int irq;
static unsigned int timer0;

int main (void) {                       /* Main Function                      */

  int num      = -1; 
  int dir      =  1;
  STATIC INT_CFG_T irqcfg = {0};

  CPLD_LED |=  LED_MSK;                 /* Turn off all LEDs                  */

  /* Setup Timer0 to generate interrupt every 10 ms                           */
  timer0 = timer0_open(TIMER0_BASE, 50803200);
  timer0_ioctl(timer0, TIMER_SET_CLK_RATE, TIMER_HCLK_DIV_128);
  timer0_ioctl(timer0, TIMER_SET_PERIODIC_MODE, 0);
  timer0_ioctl(timer0, TIMER_SET_CMP1, 3969);
  timer0_ioctl(timer0, TIMER_ENABLE_INT, TM0_INTCTRL_CMP1);
  timer_ioctl (timer0, TIMER_START, 0);

  /* Setup VIC for Timer0 interrupt                                           */
  irqcfg.init = TRUE;
  irq = (unsigned long)&irqcfg;
  irq_ioctl(irq, IRQ_ENABLE_SOURCE, VIC_TIMER0);
  irq_ioctl(irq, IRQ_SET_HANDLER, (unsigned long)TIMER0_IRQ_Handler);

  for (;;) {                            /* Loop forever                       */

    /* Calculate 'num': 0, 1, ... , LED_NUM-1, LED_NUM-1, ... , 1, 0, 0, ...  */
    num += dir;
    if (num == LED_NUM) { dir = -1; num =  LED_NUM-1; } 
    else if   (num < 0) { dir =  1; num =  0;         }

    /* Turn  on LED with index 'num'                                          */
    CPLD_LED &= ~led_mask[num];
    wait();
    /* Turn off all LEDs                                                      */
    CPLD_LED |=  LED_MSK;
    wait();
  }
}
