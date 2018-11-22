/******************************************************************************/
/* Blinky.c: LED Flasher                                                      */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2009 Keil Software. All rights reserved.                     */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/
                  
#include "MPS_CM0.H"                    /* MPS Cortex-M0 definitions          */
#include "charlcd.h"

#define LED_NUM     8                   /* Number of user LEDs                */
#define LED_ON(n)   CPU_SYS->LED |=  led_mask[n]; DUT_SYS->LED |=  led_mask[n];
#define LED_OFF(n)  CPU_SYS->LED &= ~led_mask[n]; DUT_SYS->LED &= ~led_mask[n];
#define LED_ALL_OFF CPU_SYS->LED &= ~0x000000FF;  DUT_SYS->LED &= ~0x000000FF;

const int led_mask[] = { 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80 };


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
  int idx = -1, dir = 1; 

  SystemInit();
  SysTick_Config(SystemFrequency/1000 - 1); /* Generate interrupt each 1 ms   */

	charlcd_init();
	charlcd_write_string("   MPS-CM0 Blinky   ");
	charlcd_write_string("    www.keil.com    ");

  LED_ALL_OFF                           /* Turn off all LEDs                  */
  
  for (;;) {                            /* Loop forever                       */
    /* Calculate 'idx': 0,1,...,LED_NUM-1,LED_NUM-1,...,1,0,0,...             */
    idx += dir;
    if (idx == LED_NUM) { dir = -1; idx =  LED_NUM-1; } 
    else if   (idx < 0) { dir =  1; idx =  0;         }

    LED_ON(idx)                         /* Turn on LED 'idx'                  */
    Delay(500);                         /* Delay 500ms                        */
    LED_OFF(idx)                        /* Turn off LED 'idx'                 */
    Delay(500);                         /* Delay 500ms                        */
  }
}
