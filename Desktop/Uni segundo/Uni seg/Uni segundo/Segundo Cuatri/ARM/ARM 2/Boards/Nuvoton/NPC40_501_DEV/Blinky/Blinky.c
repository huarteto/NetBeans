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
#include "NUC501.h"                          /* NUC501 settings               */


/* Import external functions */
extern void SER_init (int uart);
extern void TIM_init (int chn, int ms);
extern void TIM_start(int chn);

extern volatile uint16_t clock_1s;
                uint16_t led = 0;

int main (void)  {

  SER_init(0);                               /* UART#0 Initialization         */
  TIM_init(0, 10);                           /* Timer with 10ms tick          */

  NUC_GPIO->PORT[1].OMD  |=  (1UL << 3);     /* Port pin is out put           */
  NUC_GPIO->PORT[1].DOUT |=  (1UL << 3);     /* Port pin is high              */

  printf ("Hello World\n\r");

  TIM_start(0);                              /* Timer start                   */

  while (1) {
    if (clock_1s) {
      clock_1s = 0;

      led   ^= 1;

      if(led) {
        NUC_GPIO->PORT[1].DOUT &= ~(1UL << 3);  /* Port pin is low            */
        printf ("LED on\n\r");
      } else {
        NUC_GPIO->PORT[1].DOUT |=  (1UL << 3);  /* Port pin is high           */
        printf ("LED off\n\r");
      }
    }
  }
}
