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

  CSP_PIOInit(PIO1, 0x0F << 6, 0);      /* Setup the LED's PIO pins as Outputs*/

  /* Set Simple Timer 1 to generate interrupt every 10 ms at 10 MHz GICCLK, 
     on channel 1                                                             */
  CSP_ST16Init(ST1);
  CSP_GICInit();
  CSP_ST16ConfigInterrupt(ST1,
                          GIC_POSITIVE_EDGE_TRIGGERED, 
                          ST16_CHEND1, 
                          (unsigned long)ST1_IRQ_Handler);
  CSP_ST16ConfigureChannel(ST1, 1, ST16_SYSCAL(62) | ST16_AUTOREL ,ST16_LOAD(799));	
  CSP_ST16Enable(ST1, 1);

  for (;;) {                            /* Loop forever                       */

    /* Calculate 'num': 0, 1, ... , LED_NUM-1, LED_NUM-1, ... , 1, 0, 0, ...  */
    num += dir;
    if (num == LED_NUM) { dir = -1; num =  LED_NUM-1; } 
    else if   (num < 0) { dir =  1; num =  0;         }

    /* If SW1 button pressed increment Speed                                  */
    if ((CSP_PIO_GET_PDSR(PIO1) & (1 << 10)) && (Speed < 10))
      Speed ++;
    /* If SW2 button pressed decrement Speed                                  */
    if ((CSP_PIO_GET_PDSR(PIO1) & (1 << 11)) && (Speed >  0))
      Speed --;

    CSP_PIO_SET_SODR(PIO1, led_mask[num]);  /* Turn  on LED with index 'num'  */
    wait();
    CSP_PIO_SET_CODR(PIO1, led_mask[num]);  /* Turn off LED with index 'num'  */
    wait();
  }
}
