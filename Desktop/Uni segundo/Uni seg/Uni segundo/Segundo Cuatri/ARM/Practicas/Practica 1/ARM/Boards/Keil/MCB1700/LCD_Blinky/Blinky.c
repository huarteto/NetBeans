/******************************************************************************/
/* Blinky.c: LED Flasher and graphic demo                                     */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2005-2009 Keil Software. All rights reserved.                */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/
                  
#include <LPC17xx.H>                    /* NXP LPC17xx definitions            */
#include "string.h"
#include "GLCD.h"

#define LED_NUM     8                   /* Number of user LEDs                */
const  unsigned long led_mask[] = { 1<<28, 1<<29, 1UL<<31, 1<<2, 1<<3, 1<<4, 1<<5, 1<<6 };

extern unsigned char ClockLEDOn;
extern unsigned char ClockLEDOff;
extern unsigned char ClockANI;
extern unsigned char Bg_16bpp_t[];
extern unsigned char Bg_16bpp_l[];
extern unsigned char Bg_16bpp_r[];
extern unsigned char Bg_16bpp_b[];
extern unsigned char ARM_Ani_16bpp[];

int main (void) {                       /* Main Program                       */
  int num     = -1; 
  int dir     =  1;
  int pic     =  0;
  LPC_GPIO_TypeDef *gpio = LPC_GPIO1;

  SystemInit();
  SysTick_Config(SystemFrequency/100);  /* Generate interrupt every 10 ms     */

  LPC_GPIO1->FIODIR = 0xB0000000;       /* LEDs on PORT1 defined as Output    */
  LPC_GPIO2->FIODIR = 0x0000007C;       /* LEDs on PORT2 defined as Output    */

  GLCD_Init   ();
  GLCD_Clear  (White);
  GLCD_Bmp (  0,   0, 320,  69, Bg_16bpp_t+70);
  GLCD_Bmp (  0,  69,   4, 102, Bg_16bpp_l+70);
  GLCD_Bmp (316,  69,   4, 102, Bg_16bpp_r+70);
  GLCD_Bmp (  0, 171, 320,  69, Bg_16bpp_b+70);

  for (;;) {                            /* Loop forever                       */
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
    
      if (num < 3) gpio = LPC_GPIO1;
      else         gpio = LPC_GPIO2;
    
      gpio->FIOPIN |=  led_mask[num];
    }
    if (ClockLEDOff) {
      ClockLEDOff = 0;
      gpio->FIOPIN &= ~led_mask[num];
    }
  }
}
