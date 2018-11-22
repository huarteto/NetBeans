/******************************************************************************/
/* Blinky.c: LED Flasher                                                      */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2005-2010 Keil Software. All rights reserved.                */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/


#include "a2fxxxm3.h"                   /* A2FxxxM3x definitions              */

#define LED_NUM     1                   /* Number of user LEDs                */

const unsigned long led_mask[] = { 1 << 6 };
volatile unsigned long SysTickCnt;      /* SysTick Counter                    */

void SysTick_Handler (void) {           /* SysTick Interrupt Handler (1ms)    */
  SysTickCnt++;
}

void Delay (unsigned long tick) {       /* Delay Function                     */
  unsigned long systickcnt;

  systickcnt = SysTickCnt;
  while ((SysTickCnt - systickcnt) < tick);
}


int main (void) {                       /* Main Program                       */
  int num = -1; 
  int dir =  1;

  WATCHDOG->WDOGENABLE = 0x4C6E55FA;    /* Disable the watchdog               */

  SysTick_Config(SystemCoreClock/1000); /* Generate interrupt every 1 ms      */

  GPIO->GPIO_6_CFG = 5;                 /* Setup GPIO6 as output for LED      */

  for (;;) {                            /* Loop forever                       */
    /* Calculate 'num': 0,1,...,LED_NUM-1,LED_NUM-1,...,1,0,0,...             */
    num += dir;
    if (num == LED_NUM) { dir = -1; num =  LED_NUM-1; } 
    else if   (num < 0) { dir =  1; num =  0;         }
    GPIO->GPIO_OUT &= ~led_mask[num];
    Delay(500);
    GPIO->GPIO_OUT |=  led_mask[num];
    Delay(500);
  }
}
