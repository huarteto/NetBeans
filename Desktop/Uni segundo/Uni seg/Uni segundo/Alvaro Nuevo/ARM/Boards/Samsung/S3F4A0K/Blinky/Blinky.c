/******************************************************************************/
/* BLINKY.C: LED Flasher                                                      */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2005-2007 Keil Software. All rights reserved.                */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/
                  
#include <S3F4A.H>                      /* Samsung S3F4A register definitions */

#define LED_NUM     4                   /* Number of user LEDs                */
const int led_mask[] = { 1 << 6, 1 << 7, 1 << 8, 1 << 9 }; /* LEDs DS1 to DS4 */

extern volatile          char Speed;
extern volatile unsigned char Clock;

extern __irq void ST1_IRQ_Handler (void);


void wait (void) {                      /* Wait Function                      */

  while (!Clock);
  Clock = 0;
}


int main (void) {                       /* Main Function                      */

  int num      = -1; 
  int dir      =  1;

  PIO1->ECR    =  1;                    /* Enable PIO1 clock                  */

  PIO1->OER   |= (0x0F << 6);           /* Enable PIO1 clock                  */
  PIO1->CODR  |= (0x0F << 6);           /* Turn all LEDs off                  */

  GIC->SMR[11] = (3 << 4);              /* Positive edge triggered interrupt  */
  GIC->SVR[11] = (unsigned long)ST1_IRQ_Handler;  /* IRQ routine address      */
  GIC->IECR[0] = (1 << 11);             /* Enable ST1 interrupt               */

  /* Set Simple Timer 1 to generate interrupt every 10 ms at 10 MHz GICCLK, 
     on channel 1                                                             */
  ST1->ECR     = (1UL << 31) | (1 << 1);/* Enable ST1 clock and debug         */
  ST1->CR      = 1;                     /* Reset  ST1                         */
  ST1->CHANNEL[1].PR = (62<<8) | (1<<4);/* Clock divider = 125, auto reload   */
  ST1->CHANNEL[1].CT = 800 - 1;         /* Channel 1 preload value            */
  ST1->IER     = (1 <<  1);             /* Enable channel 1 end interrupt     */
  ST1->CR      = (1 <<  9);             /* Enable and start Timer channel 1   */

  for (;;) {                            /* Loop forever                       */

    /* Calculate 'num': 0, 1, ... , LED_NUM-1, LED_NUM-1, ... , 1, 0, 0, ...  */
    num += dir;
    if (num == LED_NUM) { dir = -1; num =  LED_NUM-1; } 
    else if   (num < 0) { dir =  1; num =  0;         }

    /* If SW1 button pressed increment Speed                                  */
    if ((PIO1->PDSR & (1 << 10)) && (Speed < 10))
      Speed ++;
    /* If SW2 button pressed decrement Speed                                  */
    if ((PIO1->PDSR & (1 << 11)) && (Speed >  0))
      Speed --;

    PIO1->SODR = led_mask[num];         /* Turn  on LED with index 'num'      */
    wait();
    PIO1->CODR = led_mask[num];         /* Turn off LED with index 'num'      */
    wait();
  }
}
