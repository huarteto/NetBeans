/******************************************************************************/
/* BLINKY.C: LED Flasher                                                      */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2010 Keil Software. All rights reserved.                     */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/
                  
#include <SAM3N_CMSIS.H>                   /* ATSAM3N definitions             */

#define LED_NUM     3                      /* Number of user LEDs             */

const unsigned long led_mask[] = { 1<<23, 1<<14, 1<<25 };
volatile unsigned long SysTickCnt;         /* SysTick Counter                 */

/*----------------------------------------------------------------------------
  SysTick Handler
 *----------------------------------------------------------------------------*/
void SysTick_Handler (void) {              /* SysTick Interrupt Handler (1ms) */
  SysTickCnt++;
}

/*----------------------------------------------------------------------------
  Delay Function
 *----------------------------------------------------------------------------*/
void Delay (unsigned long tick) {          /* Delay Function                  */
  unsigned long systickcnt;

  systickcnt = SysTickCnt;
  while ((SysTickCnt - systickcnt) < tick);
}

/*----------------------------------------------------------------------------
  Function that initializes LEDs
 *----------------------------------------------------------------------------*/
void LED_init(void) {
  PIOA->PIO_PER    =  ((1<<23) |(1<<25));   /* Setup Pins PA23 PA25 for LEDs  */
  PIOA->PIO_OER    =  ((1<<23) |(1<<25));
  PIOA->PIO_PUDR   =  ((1<<23) |(1<<25));
  PIOA->PIO_OWER   =  ((1<<23) |(1<<25));

  PIOB->PIO_PER    =  ((1<<14)         );   /* Setup Pins PB14      for LEDs  */
  PIOB->PIO_OER    =  ((1<<14)         );
  PIOB->PIO_PUDR   =  ((1<<14)         );
  PIOB->PIO_OWER   =  ((1<<14)         );
}

/*----------------------------------------------------------------------------
  Function that turns on requested LED
 *----------------------------------------------------------------------------*/
void LED_On (unsigned int num) {

  if (num == 1) PIOB->PIO_CODR = led_mask[num];
  else          PIOA->PIO_CODR = led_mask[num];
}

/*----------------------------------------------------------------------------
  Function that turns off requested LED
 *----------------------------------------------------------------------------*/
void LED_Off (unsigned int num) {

  if (num == 1) PIOB->PIO_SODR = led_mask[num];
  else          PIOA->PIO_SODR = led_mask[num];
}

/*----------------------------------------------------------------------------
  Function that outputs value to LEDs
 *----------------------------------------------------------------------------*/
void LED_Out(unsigned int value) {
  int i;

  for (i = 0; i < LED_NUM; i++) {
    if (value & (1<<i)) {
      LED_On (i);
    } else {
      LED_Off(i);
    }
  }
}

/*----------------------------------------------------------------------------
  Main Program
 *----------------------------------------------------------------------------*/
int main (void) {                          /* Main Program                    */
  int num = -1; 
  int dir =  1;

  SysTick_Config(SystemCoreClock/1000);    /* Generate interrupt each 1 ms    */

  LED_init();
  LED_Out (0x00);

  for (;;) {                               /* Loop forever                    */
    /* Calculate 'num': 0,1,...,LED_NUM-1,LED_NUM-1,...,1,0,0,...             */
    num += dir;
    if (num == LED_NUM) { dir = -1; num =  LED_NUM-1; } 
    else if   (num < 0) { dir =  1; num =  0;         }

    LED_On (num);
    Delay(500);                            /* Delay 500ms                     */
    LED_Off(num);
    Delay(500);                            /* Delay 500ms                     */
  }
}
