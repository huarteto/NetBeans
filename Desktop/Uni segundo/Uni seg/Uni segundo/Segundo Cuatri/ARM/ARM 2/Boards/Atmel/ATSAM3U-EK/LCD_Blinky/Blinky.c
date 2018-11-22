/******************************************************************************/
/* Blinky.c: LED Flasher and graphic demo                                     */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2005-2009 Keil Software. All rights reserved.                */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/
                  
#include <SAM3U.H>                      /* ATSAM3U definitions                */
#include "string.h"
#include "GLCD.h"

#define LED_NUM     2                   /* Number of user LEDs                */
const unsigned long led_mask[] = { 1<<0, 1<<1 };

extern unsigned char ClockLEDOn;
extern unsigned char ClockLEDOff;
extern unsigned char ClockANI;
extern unsigned char Bg_16bpp_t_fh_fv[];
extern unsigned char Bg_16bpp_l_fh_fv[];
extern unsigned char Bg_16bpp_r_fh_fv[];
extern unsigned char Bg_16bpp_b_fh_fv[];
extern unsigned char ARM_Ani_16bpp_fh_fv[];


int main (void) {                       /* Main Program                       */
  int num     = -1; 
  int dir     =  1;
  int pic     =  0;

  SystemInit();
  SysTick_Config(SystemFrequency/100);  /* Generate interrupt every 10 ms     */

  PMC->PMC_PCER = (1 << ID_PIOB);       /* enable clock for LEDs              */

  PIOB->PIO_IDR  =                      /* Setup Pins PB0..PB1 for LEDs       */
  PIOB->PIO_OER  =
  PIOB->PIO_PER  = (PIO_PB1 | PIO_PB0);
  PIOB->PIO_SODR = (PIO_PB1 | PIO_PB0);                      /* Turn LEDs Off */

  GLCD_init   ();
  GLCD_clear  (White);

  GLCD_bitmap (  0,   0, 320,  69, Bg_16bpp_t_fh_fv);
  GLCD_bitmap (  0,  69,   4, 102, Bg_16bpp_l_fh_fv);
  GLCD_bitmap (316,  69,   4, 102, Bg_16bpp_r_fh_fv);
  GLCD_bitmap (  0, 171, 320,  69, Bg_16bpp_b_fh_fv);

  for (;;) {                            /* Loop forever                       */
    if (ClockANI) {
      ClockANI = 0;
      if (pic++ > 8) pic = 0;
      GLCD_bitmap (99, 99, 120, 45, &ARM_Ani_16bpp_fh_fv[pic*(120*45*2)]);
    }

    if (ClockLEDOn) {    /* Blink LED every 1 second (for 0.5s)*/
      ClockLEDOn  = 0;

      /* Calculate 'num': 0,1,...,LED_NUM-1,LED_NUM-1,...,1,0,0,...             */
      num += dir;
      if (num == LED_NUM) { dir = -1; num =  LED_NUM-1; } 
      else if   (num < 0) { dir =  1; num =  0;         }
    
      PIOB->PIO_CODR = led_mask[num];
    }
    if (ClockLEDOff) {
      ClockLEDOff = 0;
      PIOB->PIO_SODR = led_mask[num];
    }
  }
}
