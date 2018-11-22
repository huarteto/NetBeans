/******************************************************************************/
/* BLINKY.C: LED Flasher                                                      */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2005-2007 Keil Software. All rights reserved.                */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/
                  
#include <LH7A404.H>                    /* NXP LH7A404 register definitions   */
#include <LH7A404_LIB.H>                /* NXP LH7A404 library functions      */

#define LED_NUM     2                   /* Number of user LEDs                */
#define LED0        (1 << 6)            /* STATUS LED0                        */
#define LED1        (1 << 7)            /* STATUS LED1                        */
#define LED_MSK     (LED0 | LED1)       /* Mask of all LEDs                   */

const int led_mask[] = {LED0, LED1};    /* Status LEDs                        */

extern volatile unsigned char Clock;

extern __irq void TIMER1_IRQ_Handler (void);


void wait (void) {                      /* Wait Function                      */

  while (!Clock);
  Clock = 0;
}

static unsigned int timer1;

int main (void) {                       /* Main Function                      */

  int num      = -1; 
  int dir      =  1;

  /* Set LED pins as outputs                                                  */
  gpio_set_data_dir(GPIO_PORT_E, LED_MSK, GPIO_OUTPUT);
  gpio_data_write(GPIO_PORT_E, LED_MSK);/* Turn off all LEDs                  */

  /* Setup Timer1 to generate interrupt every 10 ms                           */
  timer1 = timer_open(TIMER1, 0);
  timer_ioctl(timer1, TIMER_CLOCK, TIMER_CLOCK_2K);
  timer_ioctl(timer1, TIMER_SET_COUNT, 2000 / 100);
  timer_ioctl(timer1, TIMER_MODE, TIMER_PERIODIC);
  timer_ioctl(timer1, TIMER_ENABLE, 1);

  /* Setup VIC1 for Timer1 interrupt                                          */
  vic_install_handler(VIC_TC1UINTR, VIC_VECTORED, (PFV)TIMER1_IRQ_Handler);
  vic_install_handler(VIC_TC1UINTR, VIC_IRQ, 0);
  vic_int_enable(VIC_TC1UINTR, TRUE);

  for (;;) {                            /* Loop forever                       */

    /* Calculate 'num': 0, 1, ... , LED_NUM-1, LED_NUM-1, ... , 1, 0, 0, ...  */
    num += dir;
    if (num == LED_NUM) { dir = -1; num =  LED_NUM-1; } 
    else if   (num < 0) { dir =  1; num =  0;         }

    /* Turn  on LED with index 'num'                                          */
    gpio_data_write(GPIO_PORT_E, gpio_data_read(GPIO_PORT_E) & ~led_mask[num]);
    wait();
    /* Turn off all LEDs                                                      */
    gpio_data_write(GPIO_PORT_E, gpio_data_read(GPIO_PORT_E) |  LED_MSK);
    wait();
  }
}
