/******************************************************************************/
/* Blinky.c: LED Flasher                                                      */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2009 Keil Software. All rights reserved.                     */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/
                  
#include "LPC13xx.h"                    /* LPC13xx definitions                */

#define LED_NUM     8                   /* Number of user LEDs                */

const unsigned long led_mask[] = { 1<<0, 1<<1, 1<<2, 1<<3, 1<<4, 1<<5, 1<<6, 1<<7 };
volatile unsigned long SysTickCnt;      /* SysTick Counter                    */

void SysTick_Handler (void) {           /* SysTick Interrupt Handler (1ms)    */
  SysTickCnt++;
}

void Delay (unsigned long tick) {       /* Delay Function                     */
  unsigned long systickcnt;

  systickcnt = SysTickCnt;
  while ((SysTickCnt - systickcnt) < tick) __wfi();
}

int main (void) {                       /* Main Program                       */
  int num = -1; 
  int dir =  1;

  SystemInit();
  LPC_SYSCON->SYSTICKCLKDIV = LPC_SYSCON->SYSAHBCLKDIV; /* Set systick clock = sys clock */
  SysTick_Config(SystemFrequency/100);  /* Generate interrupt each ~10 ms     */

  LPC_SYSCON->SSPCLKDIV  = 0;           /* Disable unused dividers            */
  LPC_SYSCON->UARTCLKDIV = 0;
  LPC_SYSCON->USBCLKDIV  = 0;
  LPC_SYSCON->WDTCLKDIV  = 0;
  LPC_SYSCON->CLKOUTDIV  = 0;

  LPC_SYSCON->PDRUNCFG   = 0x5DB;       /* Leave only Flash and Sysosc clocks */

  LPC_SYSCON->SYSAHBCLKCTRL |= 1 << 6;  /* Enable clock for GPIO              */
  LPC_GPIO2->DIR = 0x000000FF;          /* LEDs on PORT2 defined as Output    */

  for (;;) {                            /* Loop forever                       */
    /* Calculate 'num': 0,1,...,LED_NUM-1,LED_NUM-1,...,1,0,0,...             */
    num += dir;
    if (num == LED_NUM) { dir = -1; num =  LED_NUM-1; } 
    else if   (num < 0) { dir =  1; num =  0;         }
    LPC_GPIO2->DATA &= ~led_mask[num];
    Delay(5);                           /* Delay 50ms                         */
    LPC_GPIO2->DATA |=  led_mask[num];
    Delay(45);                          /* Delay 450ms                        */
  }
}
