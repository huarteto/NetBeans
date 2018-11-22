/******************************************************************************/
/* Blinky.c: LED Flasher and graphic demo                                     */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2010 Keil Software. All rights reserved.                     */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/
                  
#include <SAM3S.H>                         /* ATSAM3S definitions             */
#include "string.h"
#include "GLCD.h"

#define LED_NUM     2                      /* Number of user LEDs             */
const unsigned long led_mask[] = { 1<<20, 1<<21 };

extern unsigned char ClockLEDOn;
extern unsigned char ClockLEDOff;
extern unsigned char ClockANI;
extern unsigned char Bg_16bpp_t[];
extern unsigned char Bg_16bpp_l[];
extern unsigned char Bg_16bpp_r[];
extern unsigned char Bg_16bpp_b[];
extern unsigned char ARM_Ani_16bpp[];


int main (void) {                          /* Main Program                    */
  int num     = -1; 
  int dir     =  1;
  int pic     =  0;

  SystemInit();
  SysTick_Config(SystemFrequency/100);     /* Generate interrupt every 10 ms  */

  PMC->PMC_WPMR = 0x504D4300;              /* Disable write protect           */
  PMC->PMC_PCER1 = (1 << ID_PIOC);         /* enable clock for LEDs           */

  PIOC->PIO_IDR  =                         /* Setup Pins PC20..PC21 for LEDs  */
  PIOC->PIO_OER  =
  PIOC->PIO_PER  = ((1<<20) |(1<<21));
  PIOC->PIO_SODR = ((1<<20) |(1<<21));                       /* Turn LEDs Off */

  GLCD_Init   ();
  GLCD_Clear  (White);
  GLCD_Bmp (  0,   0, 320,  69, Bg_16bpp_t+70);
  GLCD_Bmp (  0,  69,   4, 102, Bg_16bpp_l+70);
  GLCD_Bmp (316,  69,   4, 102, Bg_16bpp_r+70);
  GLCD_Bmp (  0, 171, 320,  69, Bg_16bpp_b+70);

  for (;;) {                               /* Loop forever                    */
    if (ClockANI) {
      ClockANI = 0;
      if (pic++ > 8) pic = 0;
        GLCD_Bmp (99, 99, 120, 45, &ARM_Ani_16bpp[pic*(120*45*2)]);
    }

    if (ClockLEDOn) {    /* Blink LED every 1 second (for 0.5s)*/
      ClockLEDOn  = 0;

      /* Calculate 'num': 0,1,...,LED_NUM-1,LED_NUM-1,...,1,0,0,...           */
      num += dir;
      if (num == LED_NUM) { dir = -1; num =  LED_NUM-1; } 
      else if   (num < 0) { dir =  1; num =  0;         }
    
      PIOC->PIO_CODR = led_mask[num];
    }
    if (ClockLEDOff) {
      ClockLEDOff = 0;
      PIOC->PIO_SODR = led_mask[num];
    }
  }
}
