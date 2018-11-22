/******************************************************************************/
/* BLINKY.C: LED Flasher                                                      */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2010 Keil Software. All rights reserved.                     */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/
                  
#include <SAM3S.H>                         /* ATSAM3S definitions             */

#define LED_NUM     2                      /* Number of user LEDs             */

const unsigned long led_mask[] = { 1<<20, 1<<21 };
volatile unsigned long SysTickCnt;         /* SysTick Counter                 */

void SysTick_Handler (void) {              /* SysTick Interrupt Handler (1ms) */
  SysTickCnt++;
}

void Delay (unsigned long tick) {          /* Delay Function                  */
  unsigned long systickcnt;

  systickcnt = SysTickCnt;
  while ((SysTickCnt - systickcnt) < tick);
}

int main (void) {                          /* Main Program                    */
  int num = -1; 
  int dir =  1;

  SystemInit();
  SysTick_Config(SystemFrequency/1000);    /* Generate interrupt each 1 ms    */

  PIOC->PIO_PER    =  ((1<<20) |(1<<21));  /* Setup Pins PC19..PC20 for LEDs  */
  PIOC->PIO_OER    =  ((1<<20) |(1<<21));
  PIOC->PIO_PUDR   =  ((1<<20) |(1<<21));
  PIOC->PIO_OWER   =  ((1<<20) |(1<<21));

  for (;;) {                               /* Loop forever                    */
    /* Calculate 'num': 0,1,...,LED_NUM-1,LED_NUM-1,...,1,0,0,...             */
    num += dir;
    if (num == LED_NUM) { dir = -1; num =  LED_NUM-1; } 
    else if   (num < 0) { dir =  1; num =  0;         }

    PIOC->PIO_CODR = led_mask[num];
    Delay(500);                            /* Delay 500ms                     */
    PIOC->PIO_SODR = led_mask[num];
    Delay(500);                            /* Delay 500ms                     */
  }
}
