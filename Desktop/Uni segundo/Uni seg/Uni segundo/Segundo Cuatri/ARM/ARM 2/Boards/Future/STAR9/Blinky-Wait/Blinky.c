/******************************************************************************/
/* BLINKY.C: LED Flasher for Star9 Board                                      */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2005-2007 Keil Software. All rights reserved.                */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/

#include <91x_lib.h> 

#define LED_RED   0x04
#define LED_YEL   0x40

#define LED_On(led)         GPIO2->DR[led*4] = ~led
#define LED_Off(led)        GPIO2->DR[led*4] =  led
#define LED_out(mask, val)  GPIO2->DR[mask*4] = ~(val & mask)

 /* Blink the LEDs on Star9 board */
 const unsigned char led_val[4] = { 0x04,0x44,0x40,0x00 };
 static unsigned int led_cnt = 0;


void wait (void)  {                       /* Wait function                    */
  unsigned long i;

  for (i = 0; i < 350000; i++);           /* Only to delay for LED flashes    */
}


int main (void) {

  /* init LEDs */
  SCU->GPIOOUT[2]   = 0x1010;             /* P2.2, P2.6 output - mode 1       */
  GPIO2->DDR        = 0x44;               /* P2.2, P2.6 Outputs (LED Data)    */
  GPIO2->DR[0x44*4] = 0x44;	              /* clear LEDs                       */

  while (1) {                             /* Loop forever                     */
    LED_out (0x44, led_val[led_cnt]);
    if (++led_cnt >= sizeof(led_val)) {
      led_cnt = 0;
    }
    wait();                               /* Delay                            */
  }
}
