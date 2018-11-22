/******************************************************************************/
/* Blinky.c: LED Flasher                                                      */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2010 Keil Software. All rights reserved.                     */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/

#include <stdio.h>
#include "BTN.h"                        /* Buttons functions prototypes       */
#include "LED.h"                        /* LED functions prototypes           */
#include "ADC.h"                        /* ADC functions prototypes           */
#include "TMPM364.h"                    /* TMPM364 definitions                */


#define LED_NUM     4                   /* Number of user LEDs                */


volatile unsigned short AD_Val;         /* AD Value                           */
volatile unsigned long  SysTickCnt;     /* SysTick Counter                    */
volatile unsigned long  ClockFlag;      /* Clock tick flag                    */


void SysTick_Handler (void) {           /* SysTick Interrupt Handler (1ms)    */
  SysTickCnt++;

  if (SysTickCnt >= ((AD_Val>>3)+10)) {
    SysTickCnt = 0;
    ClockFlag  = 1;
    AD_Val     = ADC_Get();             /* Get AD conversion                  */
  }
}


int main (void) {                       /* Main Program                       */
  int idx  = -1, dir  = 1,  up = 1;
  int leds =  0, btns = 0;

  BTN_Init();                           /* Initialize buttons                 */
  LED_Init();                           /* Initialize LEDs                    */
  ADC_Init();                           /* Initialize ADC                     */
  
  SysTick_Config(SystemCoreClock/1000); /* Generate interrupt every 1 ms      */

  for (;;) {                            /* Loop forever                       */
    btns = BTN_Get();                   /* Read button states                 */

    if (ClockFlag) {
      ClockFlag = 0;

      if (up) {
        /* Calculate 'idx': 0,1,...,LED_NUM-1,LED_NUM-1,...,1,0,0,...         */
        idx += dir;
        if (idx == LED_NUM) { dir = -1; idx =  LED_NUM-1; } 
        else if   (idx < 0) { dir =  1; idx =  0;         }

        leds |=  (1 << idx);
      } else {
        leds &= ~(1 << idx);
      }
      up ^= 1;
    }

    LED_Out(leds | btns);               /* Activate LEDs                      */
  }
}
