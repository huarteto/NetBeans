/******************************************************************************/
/* BLINKY.C: LED Flasher                                                      */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2009 Keil Software. All rights reserved.                     */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/
                  
#include <SAM3U.H>                      /* ATSAM3U definitions                */

#define LED_NUM     2                   /* Number of user LEDs                */

#if   defined FLASH_BANK && (FLASH_BANK == 0)
  const unsigned long led_mask[] = { 1<<0, 1<<0 };
#elif defined FLASH_BANK && (FLASH_BANK == 1)
  const unsigned long led_mask[] = { 1<<1, 1<<1 };
#else
  const unsigned long led_mask[] = { 1<<0, 1<<1 };
#endif

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

  SystemInit();
  SysTick_Config(SystemFrequency/1000 - 1); /* Generate interrupt each 1 ms   */

  PIOB->PIO_PER    =  0x03;             /* Setup Pins PB0..PB1 for LEDs       */
  PIOB->PIO_OER    =  0x03;
  PIOB->PIO_PUDR   =  0x03;
  PIOB->PIO_OWER   =  0x03;
  PIOB->PIO_ABSR  &= ~0x03;

  for (;;) {                            /* Loop forever                       */
    /* Calculate 'num': 0,1,...,LED_NUM-1,LED_NUM-1,...,1,0,0,...             */
    num += dir;
    if (num == LED_NUM) { dir = -1; num =  LED_NUM-1; } 
    else if   (num < 0) { dir =  1; num =  0;         }

    PIOB->PIO_CODR = led_mask[num];
    Delay(500);                         /* Delay 500ms                        */
    PIOB->PIO_SODR = led_mask[num];
    Delay(500);                         /* Delay 500ms                        */
  }
}
