/******************************************************************************/
/* Blinky.c: Blinky program                                                   */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2009-2010 KEIL - An ARM Company. All rights reserved.        */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/
                  
#include <stdio.h>
#include "BU1511KV2.h"                    /* BU1511KV2 settings               */


/* Import external functions */
extern void SER_init (int uart);
extern void TIM_init (int timer, int ms);
extern void TIM_start(int timer);

extern volatile uint16_t clock_1s;
                uint16_t led = 0;

int main (void)  {

  SER_init(2);                            /* UART#2 Initialization            */
  TIM_init(1, 10);                        /* Timer1 with 10ms tick            */

  ROHM_GPIO->swporta_ddr |=  (1 << 0);    /* GPIO0 is output                  */
  ROHM_GPIO->swporta_dr  |=  (1 << 0);    /* port is HIGH                     */

  printf ("Hello World\n\r");

  TIM_start(1);                           /* Timer#1 start                    */

  while (1) {
    if (clock_1s) {
      clock_1s = 0;

      led   ^= 1;

      if(led) {
        ROHM_GPIO->swporta_dr  &= ~(1 << 0);    /* set GPIO port 0 LOW        */
        printf ("LED on\n\r");
      } else {
        ROHM_GPIO->swporta_dr  |=  (1 << 0);    /* set GPIO port 0 HIGH       */
        printf ("LED off\n\r");
      }
    }
  }
}
